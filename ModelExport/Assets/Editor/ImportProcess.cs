using UnityEditor;

namespace Editor
{
	public class ImportProcess : AssetPostprocessor
	{
		public override int GetPostprocessOrder()
		{
			return 0;
		}

		private void OnPreprocessTexture()
		{
			var textureImporter = assetImporter as TextureImporter;

			// ReSharper disable once PossibleNullReferenceException
			textureImporter.isReadable = true;
			textureImporter.SetPlatformTextureSettings(new TextureImporterPlatformSettings
			{
				overridden = true,
				name = "Standalone",
				format = TextureImporterFormat.RGBA32
			});
		}
	}
}
