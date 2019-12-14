#define kDieletricSpec half4(0.04, 0.04, 0.04, 1.0 - 0.04) // standard dielectric reflectivity coef at incident angle (= 4%)

struct InputData
{
    float3 normalWS;
    float3 viewDirectionWS;
};

half PerceptualSmoothnessToPerceptualRoughness(half perceptualSmoothness)
{
    return (1.0 - perceptualSmoothness);
}

half PerceptualRoughnessToRoughness(half perceptualRoughness)
{
    return perceptualRoughness * perceptualRoughness;
}

// Abstraction over Light shading data.
struct Light
{
    half3 direction;
    half3 color;
    half distanceAttenuation;
    half shadowAttenuation;
};

///////////////////////////////////////////////////////////////////////////////
//                         BRDF Functions                                    //
///////////////////////////////////////////////////////////////////////////////

struct BRDFData
{
    half3 diffuse;
    half3 specular;
    half perceptualRoughness;
    half roughness;
    half roughness2;
    half grazingTerm;

    // We save some light invariant BRDF terms so we don't have to recompute
    // them in the light loop. Take a look at DirectBRDF function for detailed explaination.
    half normalizationTerm; // roughness * 4.0 + 2.0
    half roughness2MinusOne; // roughness² - 1.0
};

half ReflectivitySpecular(half3 specular)
{
    return max(max(specular.r, specular.g), specular.b);
}

half OneMinusReflectivityMetallic(half metallic)
{
    // We'll need oneMinusReflectivity, so
    //   1-reflectivity = 1-lerp(dielectricSpec, 1, metallic) = lerp(1-dielectricSpec, 0, metallic)
    // store (1-dielectricSpec) in kDieletricSpec.a, then
    //   1-reflectivity = lerp(alpha, 0, metallic) = alpha + metallic*(0 - alpha) =
    //                  = alpha - metallic * alpha
    half oneMinusDielectricSpec = kDieletricSpec.a;
    return oneMinusDielectricSpec - metallic * oneMinusDielectricSpec;
}

void InitializeBRDFData(half3 albedo, half metallic, half smoothness, half alpha, out BRDFData outBRDFData)
{
    half oneMinusReflectivity = OneMinusReflectivityMetallic(metallic);
    half reflectivity = 1.0 - oneMinusReflectivity;

    outBRDFData.diffuse = albedo * oneMinusReflectivity;
    outBRDFData.specular = lerp(kDieletricSpec.rgb, albedo, metallic);

    outBRDFData.grazingTerm = saturate(smoothness + reflectivity);
    outBRDFData.perceptualRoughness = PerceptualSmoothnessToPerceptualRoughness(smoothness);
    outBRDFData.roughness = PerceptualRoughnessToRoughness(outBRDFData.perceptualRoughness);
    outBRDFData.roughness2 = outBRDFData.roughness * outBRDFData.roughness;

    outBRDFData.normalizationTerm = outBRDFData.roughness * 4.0h + 2.0h;
    outBRDFData.roughness2MinusOne = outBRDFData.roughness2 - 1.0h;
}

half3 DirectBDRF(BRDFData brdfData, half3 normalWS, half3 lightDirectionWS, half3 viewDirectionWS)
{
    half3 halfDir = normalize(lightDirectionWS + viewDirectionWS);

    half NoH = saturate(dot(normalWS, halfDir));
    half LoH = saturate(dot(lightDirectionWS, halfDir));

    // Final BRDFspec = roughness² / ( NoH² * (roughness² - 1) + 1 )² * (LoH² * (roughness + 0.5) * 4.0)
    // We further optimize a few light invariant terms
    // brdfData.normalizationTerm = (roughness + 0.5) * 4.0 rewritten as roughness * 4.0 + 2.0 to a fit a MAD.
    half d = NoH * NoH * brdfData.roughness2MinusOne + 1.00001h;

    half LoH2 = LoH * LoH;
    half specularTerm = brdfData.roughness2 / ((d * d) * max(0.1h, LoH2) * brdfData.normalizationTerm);


    half3 color = specularTerm * brdfData.specular + brdfData.diffuse;
    return color;
}

half3 LightingPhysicallyBased(BRDFData brdfData, half3 lightColor, half3 lightDirectionWS, half lightAttenuation, half3 normalWS, half3 viewDirectionWS)
{
    half NdotL = saturate(dot(normalWS, lightDirectionWS));
    half3 radiance = lightColor * (lightAttenuation * NdotL);
    return DirectBDRF(brdfData, normalWS, lightDirectionWS, viewDirectionWS) * radiance;
}

half3 LightingPhysicallyBased(BRDFData brdfData, Light light, half3 normalWS, half3 viewDirectionWS)
{
    return LightingPhysicallyBased(brdfData, light.color, light.direction, light.distanceAttenuation * light.shadowAttenuation, normalWS, viewDirectionWS);
}

half4 LightweightFragmentPBR(InputData inputData, half3 albedo, half metallic,
    half smoothness, half occlusion, half3 emission, half alpha)
{
    BRDFData brdfData;
    InitializeBRDFData(albedo, metallic, smoothness, alpha, brdfData);

    Light mainLight = (Light)0;
    mainLight.direction = -normalize(half3(0.5, 0.5, 0));
    mainLight.color = half3(1, 1, 1);
    mainLight.distanceAttenuation = mainLight.shadowAttenuation = 1;

    float3 color = LightingPhysicallyBased(brdfData, mainLight, inputData.normalWS, inputData.viewDirectionWS);
    mainLight.direction = -mainLight.direction;
    color += LightingPhysicallyBased(brdfData, mainLight, inputData.normalWS, inputData.viewDirectionWS);
    mainLight.direction = normalize(half3(0, 1, 0));
    color += LightingPhysicallyBased(brdfData, mainLight, inputData.normalWS, inputData.viewDirectionWS);

#ifdef _ADDITIONAL_LIGHTS
    int pixelLightCount = GetAdditionalLightsCount();
    for (int i = 0; i < pixelLightCount; ++i)
    {
        Light light = GetAdditionalLight(i, inputData.positionWS);
        color += LightingPhysicallyBased(brdfData, light, inputData.normalWS, inputData.viewDirectionWS);
    }
#endif

#ifdef _ADDITIONAL_LIGHTS_VERTEX
    color += inputData.vertexLighting * brdfData.diffuse;
#endif

    color += emission;
    return half4(color, alpha);
}