#include "model.h"
#include "animation.h"
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	// =======================================================
	// エントリポイント
	// -------------------------------------------------------
	// コマンドライン引数に応じて
	// モデル or アニメーションのコンバータを実行する
	//
	// 使用方法：
	// FBXTool <mode> <input> <output>
	//
	// mode:
	//   "model"     → モデル変換（.mgm）
	//   "animation" → アニメーション変換（.mga）
	// =======================================================

	// 引数チェック（mode / input / output の3つが必要）
	if (argc < 4) {
		std::cerr << "Usage: FBXTool <mode> <input> <output>\n";
		return 1;
	}

	// モード取得（model / animation）
	std::string mode = argv[1];

	// 入力ファイル（FBXなど）
	const char* input = argv[2];

	// 出力ファイル（.mgm / .mga）
	const char* output = argv[3];

	fs::path inputPath(input);

	// 入力ファイルが存在する場合のみ処理
	if (fs::exists(inputPath)) {

		// =======================================================
		// モデル変換
		// =======================================================
		if (mode == "model") {
			ReadModel(input, output);

			std::cout << "Model Input : " << input << "\n";
			std::cout << "Model Output: " << output << "\n";
		}

		// =======================================================
		// アニメーション変換
		// =======================================================
		else if (mode == "animation") {
			ReadAnimation(input, output);

			std::cout << "Animation Input : " << input << "\n";
			std::cout << "Animation Output: " << output << "\n";
		}
	}

	return 0;
}