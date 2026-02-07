#include "csvResource.h"
#include <fstream>
#include <sstream>

namespace MG {

	CSVResource::String::operator Vector3() const
	{
		float result[3] = { 0.0f, 0.0f, 0.0f };
		auto values = CSVResource::ParseCSVLine(str);
		for (int i = 0; i < 3 && i < values.size(); i++) {
			result[i] = std::stof(values[i]);
		}
		return { result[0], result[1], result[2] };
	}

	CSVResource::String::operator Vector4() const
	{
		if (str.c_str()[0] == '#') {
			unsigned long hex = std::stoul(str, nullptr, 16);
			return Vector4{ (float)(hex >> 24 & 0xFF), (float)(hex >> 16 & 0xFF), (float)(hex >> 8 & 0xFF), (float)(hex & 0xFF) } / 255.0f;
		}
		else
		{
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
		std::string line;
		unsigned int rowNum = 0;
		unsigned int colNum = 0;

		while (std::getline(stream, line)) {
			unsigned int rowColNum = 0;

			auto cells = ParseCSVLine(line);
			for (auto& cell : cells) {
				if (rowNum == 0) m_ColKeys[cell] = rowColNum;
				if (rowColNum == 0) m_RowKeys[cell] = rowNum;
				m_Data.push_back(cell);
				rowColNum++;
			}

			colNum = std::max(colNum, rowColNum);
			m_RowViews.push_back({ this, rowNum });
			rowNum++;
		}

		m_ColCount = colNum;
	}

	std::vector<std::string> CSVResource::ParseCSVLine(const std::string& line)
	{
		std::vector<std::string> result;
		std::string cell;
		bool inQuotes = false;

		for (size_t i = 0; i < line.size(); ++i) {
			char c = line[i];

			// '\r'–³Ž‹
			if (c == '\r') {
				continue;
			}

			if (c == '"') {
				if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
					cell += '"';
					++i;
				}
				else {
					inQuotes = !inQuotes;
				}
			}
			else if (c == ',' && !inQuotes) {
				result.push_back(cell);
				cell.clear();
			}
			else {
				cell += c;
			}
		}

		result.push_back(cell);
		return result;
	}

	CSVResource::CSVResource(const char* filename)
	{
		std::ifstream file(filename);
		std::string line;
		if (file.is_open()) {
			Parse(file);
			file.close();
		}
	}

	CSVResource::CSVResource(const unsigned char* data, size_t size)
	{
		std::string text(reinterpret_cast<const char*>(data), size);
		std::istringstream stream(text);
		Parse(stream);
	}

	

} // namespace MG