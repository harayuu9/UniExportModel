# UniExportModel
##Supported version 
Unity 2018 or later<br>
VC++ and WindowsSDK
## What is it?
UnityからModel情報を掃き出しC++で読み込むことが出来る。<br>
Unityで作ったシーン情報をMaterialデータを含む全てを外部アプリケーションで使用することが出来ます<br>

## Quick Start
このリポジトリをダウンロードする。<br>
###Unity
`UniExportModel/ModelExport`をUnityで開く<br>
![1](https://user-images.githubusercontent.com/24310162/71397002-7a127500-265f-11ea-9660-8b19aa4e919d.png)<br>
`ModelExport/Mesh`を選択<br>

![2](https://user-images.githubusercontent.com/24310162/71398491-ec855400-2663-11ea-827e-8b3291c88079.png)<br>
`Write`...Meshに登録したオブジェクトを全てASCII形式で保存する<br>
`WriteScene`...Scene上にあるMeshRendererのオブジェクト全てをASCII形式で保存する<br>
`WriteBinary`...Meshに登録したオブジェクトを全てBinary形式で保存する<br>
`WriteSceneBinary`...Scene上にあるMeshRendererのオブジェクト全てをBinary形式で保存する<br><br>
*Material Setting*<br>
`Texture`...マテリアルのテクスチャ名<br>
`Color`...マテリアルのColor名<br><br>
*Advance Setting*<br>
吐き出す頂点データ。C++の方の型と合わせてデータを出力する<br>
共に初期設定は画像の通り<br>

###C++
Sampleの動くバージョンはVisualStudio2019になります<br>
`UniExportModel/DirectX/DirectX11.sln`をVisualStudio2019で開く<br>
メインモジュールは`UniExportModel.hpp`になります<br>
`uem::Model<T> uem::SkinnedModel<T>`...Unity側で掃き出し指定したVertexFormatが入るデータ型を指定する<br>
`uem::SkinnedAnimation`...Animation読み込み用クラス<br>
`LoadAscii(std::string filename) LoadBinary(std::string filename)`...読み込むファイルを指定して読み込み<br>

## Samples
![Unity](https://user-images.githubusercontent.com/24310162/70852954-0a77e980-1eeb-11ea-812f-8640c29b6fe2.png)<br>
<br>
![DirectX11](https://user-images.githubusercontent.com/24310162/70852958-1bc0f600-1eeb-11ea-8665-bd996535c37a.png)<br>
<br>
![Animation](https://user-images.githubusercontent.com/24310162/70852963-27142180-1eeb-11ea-86b9-4fb2efe55390.gif)<br>

## License
MIT
