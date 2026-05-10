// =======================================================
// MGResource
// -------------------------------------------------------
// 複数のバイナリファイルを1つにまとめて管理するための
// シンプルなリソースパッケージクラス
//
// ・ファイルをメモリにロードして管理（ファイル名 → データ）
// ・複数ファイルを1つのバイナリへパック（Write）
// ・パック済みファイルの展開（Load）
// ・個別ファイルの追加 / 削除 / 取得
//
// ■ ファイルフォーマット
// [HEADER]
// [ENTRY配列（ファイル名 + サイズ）]
// [各ファイルの生データ]
//
// ※データはENTRY順に連続格納される
//
// ■ 用途
// ・シェーダ / コンフィグなどのまとめ読み込み
// ・アセットの簡易パッケージ化
// ・実行時ファイルアクセス削減
//
// ※圧縮・暗号化は未対応
// =======================================================

#include "MGResource.h"
#include <fstream>
#include <vector>
#include <windows.h>

namespace MG {

	MGResource::MGResource(const char* filename)
	{
		// 指定されたリソースファイルを読み込む
		Load(filename);
	}

	void MGResource::Add(const char* filename, const char* rename)
	{
		// 追加するファイルをバイナリで開く
		// ios::ate により、開いた直後に末尾へ移動してサイズを取得しやすくする
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (file.is_open()) {
			// ファイルサイズ取得
			size_t size = static_cast<size_t>(file.tellg());

			// 読み込み位置を先頭へ戻す
			file.seekg(0, std::ios::beg);
			file.clear();

			// ファイル内容をメモリへ読み込む
			ResourceFile resFile{
				new unsigned char[size],
				size
			};

			file.read(reinterpret_cast<char*>(resFile.data), size);

			// 登録名を決定
			// rename が指定されていれば、元ファイル名ではなく別名で登録する
			std::string name = filename;
			if (rename) {
				name = rename;
			}

			// 同じ名前のファイルが既にある場合は上書きする
			if (m_Files.count(name) > 0) {
				delete[] m_Files[name].data;
				m_Files[name].size = 0;
			}

			// メモリ上のリソース一覧に登録
			m_Files[name] = resFile;

			file.close();
		}
	}

	void MGResource::Remove(const char* filename)
	{
		// 指定ファイルが登録されていればメモリ解放して削除
		if (m_Files.count(filename) > 0) {
			delete[] m_Files[filename].data;
			m_Files.erase(filename);
		}
	}

	MGResource::ResourceFile MGResource::GetFile(const char* filename)
	{
		// 登録済みファイルを取得
		if (m_Files.count(filename) > 0) {
			return m_Files[filename];
		}

		// 見つからなければ空データを返す
		return {};
	}

	void MGResource::Load(const char* filename)
	{
		// 既存リソースを一度解放してから読み込む
		Release();

		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (file) {
			// ファイル全体サイズ
			size_t size = static_cast<size_t>(file.tellg());
			file.seekg(0, std::ios::beg);

			// =======================================================
			// ヘッダ読み込み
			// =======================================================
			MG_RESOURCE_HEADER header;
			file.read(reinterpret_cast<char*>(&header), sizeof(MG_RESOURCE_HEADER));

			// =======================================================
			// エントリー情報読み込み
			// 各リソースの名前・サイズなどを取得する
			// =======================================================
			std::vector<MG_RESOURCE_ENTRY> entries;
			entries.reserve(header.entryCount);

			for (unsigned int i = 0; i < header.entryCount; i++) {
				MG_RESOURCE_ENTRY entry{};
				file.read(reinterpret_cast<char*>(&entry), sizeof(MG_RESOURCE_ENTRY));
				entries.push_back(entry);
			}

			// =======================================================
			// 実データ読み込み
			// エントリー順に並んでいるバイナリデータをメモリへ展開する
			// =======================================================
			for (auto entry : entries) {
				ResourceFile resfile{
					new unsigned char[entry.size],
					entry.size
				};

				file.read(reinterpret_cast<char*>(resfile.data), entry.size);

				// entry.name をキーとして登録
				m_Files[entry.name] = resfile;
			}

			file.close();
		}
	}

	void MGResource::Write(const char* filename)
	{
		// =======================================================
		// ヘッダ作成
		// =======================================================
		MG_RESOURCE_HEADER header{};

		// バージョン文字列
		strcpy_s(header.version, sizeof(char) * ARRAYSIZE(header.version), VERSION);

		// 登録ファイル数
		header.entryCount = static_cast<unsigned int>(m_Files.size());

		// =======================================================
		// エントリー作成
		// 各ファイルの名前とサイズを先に書き出す
		// =======================================================
		std::vector<MG_RESOURCE_ENTRY> entries;
		entries.reserve(header.entryCount);

		for (auto pair : m_Files) {
			MG_RESOURCE_ENTRY entry{};

			// リソース名
			strcpy_s(entry.name, sizeof(char) * ARRAYSIZE(entry.name), pair.first.c_str());

			// データサイズ
			entry.size = pair.second.size;

			entries.push_back(entry);
		}

		// =======================================================
		// 出力
		// ファイル構成：
		// [Header][Entry配列][各ファイルのバイナリデータ]
		// =======================================================
		std::ofstream file(filename, std::ios::binary);

		file.write((const char*)&header, sizeof(MG_RESOURCE_HEADER));
		file.write((const char*)entries.data(), sizeof(MG_RESOURCE_ENTRY) * entries.size());

		// 実データを書き出す
		for (auto pair : m_Files) {
			file.write((const char*)pair.second.data, pair.second.size);
		}

		file.close();
	}

	void MGResource::Release()
	{
		// 登録済みリソースのメモリをすべて解放
		for (auto pair : m_Files) {
			auto file = pair.second;
			delete[] file.data;
		}

		m_Files.clear();
	}


} // namespace MG