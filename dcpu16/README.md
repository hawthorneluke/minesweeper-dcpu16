### DCPU16エミュレータ
- C++
- Windows用、SDL使用
- VC++2013を利用
- その他のハードウェアのモニタ、キーボード、クロックのハードウェアも実装

## 実行方法
- ReleaseかDebugディレクトリ内のdcpu-16-Win32.exeをコマンドプロンプトから実行する。
- この時、引数でロードしたいプログラムのパスを指定する。

### Debugキー(グラフィックが出ているウィンドウを選択時)
- F1: 一時停止。一時停止中は1つのCPUサイクルを実行。
- F2: 一時停止から再生。
- F3: 1秒間のCPUサイクル数の制限を切り替え。

###　その他
- 以下specsディレクトリ内
- DCPU-16 Specification.txt: DCPU16の仕様
- 以下他のハードウェアの仕様
- Monitor Specification.txt: キーボードの仕様
- Keyboard Specification.txt: キーボードの仕様
- Clock Specification.txt: クロックの仕様
