#include "csvResource.h"
#include <fstream>
#include <sstream>

namespace MG {

	CSVResource::String::operator Vector3() const
	{
		// "x,y,z"形式の文字列をVector3へ変換する
		float result[3] = { 0.0f, 0.0f, 0.0f };

		auto values = CSVResource::ParseCSVLine(str);

		for (int i = 0; i < 3 && i < values.size(); i++) {
			result[i] = std::stof(values[i]);
		}

		return { result[0], result[1], result[2] };
	}

	CSVResource::String::operator Vector4() const
	{
		// "#RRGGBBAA"形式ならカラー値として扱う
		if (str.c_str()[0] == '#') {
			unsigned long hex = std::stoul(str, nullptr, 16);

			return Vector4{
				(float)(hex >> 24 & 0xFF),
				(float)(hex >> 16 & 0xFF),
				(float)(hex >> 8 & 0xFF),
				(float)(hex & 0xFF)
			} / 255.0f;
		}
		else
		{
			// "x,y,z,w"形式の文字列をVector4へ変換する
			float result[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

			auto values = CSVResource::ParseCSVLine(str);

			for (int i = 0; i < 4 && i < values.size(); i++) {
				result[i] = std::stof(values[i]);
			}

			return { result[0], result[1], result[2], result[3] };
		}
	}

	void CSVResource::Parse(std::istream& stream)
	{
		// CSV全体を読み込み、内部のフラット配列へ展開する
		std::string line;
		unsigned int rowNum = 0;
		unsigned int colNum = 0;

		while (std::getline(stream, line)) {
			unsigned int rowColNum = 0;

			// 1行をセル単位に分解
			auto cells = ParseCSVLine(line);

			for (auto& cell : cells) {
				// 1行目を列キーとして登録
				if (rowNum == 0) {
					m_ColKeys[cell] = rowColNum;
				}

				// 1列目を行キーとして登録
				if (rowColNum == 0) {
					m_RowKeys[cell] = rowNum;
				}

				// データは1次元配列に詰める
				m_Data.push_back(cell);
				rowColNum++;
			}

			// 最大列数を保持
			colNum = std::max(colNum, rowColNum);

			// 行ビューを作成
			m_RowViews.push_back({ this, rowNum });
			rowNum++;
		}

		m_ColCount = colNum;
	}

	std::vector<std::string> CSVResource::ParseCSVLine(const std::string& line)
	{
		// CSVの1行をセル配列へ分解する
		// クォート内のカンマ、""によるダブルクォートエスケープに対応
		std::vector<std::string> result;
		std::string cell;
		bool inQuotes = false;

		for (size_t i = 0; i < line.size(); ++i) {
			char c = line[i];

			// Windows改行の'\r'は無視
			if (c == '\r') {
				continue;
			}

			if (c == '"') {
				if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
					// クォート内の "" は " として扱う
					cell += '"';
					++i;
				}
				else {
					// クォート範囲の開始/終了を切り替える
					inQuotes = !inQuotes;
				}
			}
			else if (c == ',' && !inQuotes) {
				// クォート外のカンマでセル区切り
				result.push_back(cell);
				cell.clear();
			}
			else {
				// 通常文字として追加
				cell += c;
			}
		}

		// 最後のセルを追加
		result.push_back(cell);

		return result;
	}

	CSVResource::CSVResource(const char* filename)
	{
		// ファイルパスからCSVを読み込む
		std::ifstream file(filename);
		std::string line;

		if (file.is_open()) {
			Parse(file);
			file.close();
		}
	}

	CSVResource::CSVResource(const unsigned char* data, size_t size)
	{
		// メモリ上のCSVデータから読み込む
		// MGResourceなどでパック済みデータを扱う場合に使う
		std::string text(reinterpret_cast<const char*>(data), size);
		std::istringstream stream(text);

		Parse(stream);
	}

} // namespace MG