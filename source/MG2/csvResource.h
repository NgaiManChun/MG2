// =======================================================
// csvResource.h
// 
// CSVファイルを読み込みクラス
// 
// 使用例：
// 0,  A,  B,  C
// D, 11, 12, 13
// E, 21, 22, 23
// F, 31, 32, 33
// 
// csv[0][3]	 → "C"
// csv["E"][3]	 → "23"
// csv[3]["B"]	 → "32"
// csv["F"]["C"] → "33"
// 
// for(RowView row : csv){
//	row["B"] → "B", "12", "22", "32"
// }
// 
// for(RowView row : csv.WithoutHeaderView()){
//	row["A"] → "11", "21", "31"
// }
// =======================================================
#pragma once
#include <string>
#include <map>
#include <vector>
#include "vector4.h"

namespace MG {

	class CSVResource {
	public:

		// 型変換用のインタフェース
		struct String {
			std::string& str;

			const std::string& GetString() const
			{
				return str;
			}

			const char* GetCharArray() const
			{
				return str.data();
			}

			int GetInteger() const
			{
				return std::stoi(str);
			}

			unsigned int GetUINT() const
			{
				return std::stoul(str);
			}

			float GetFloat() const
			{
				return std::stof(str);
			}

			operator const std::string&() const
			{
				return GetString();
			}

			operator const char* () const
			{
				return GetCharArray();
			}

			operator int() const
			{
				return GetInteger();
			}

			operator unsigned int() const
			{
				return GetUINT();
			}

			operator float() const
			{
				return GetFloat();
			}

			operator Vector3() const;

			operator Vector4() const;

		};

		// 行インタフェース
		class RowView {
			friend CSVResource;

		private:
			CSVResource* m_Resource;
			size_t m_RowNum;
			RowView(CSVResource* resource, size_t rowNum): 
				m_Resource(resource), m_RowNum(rowNum) {}

		public:
			auto begin()
			{
				return m_Resource->m_Data.begin() + m_RowNum * m_Resource->m_ColCount;
			}

			auto end()
			{
				return begin() + m_Resource->m_ColCount;
			}

			String operator[](const int i)
			{
				return { m_Resource->m_Data[m_RowNum * m_Resource->m_ColCount + i] };
			}

			String operator[](std::string key)
			{
				return { m_Resource->m_Data[m_RowNum * m_Resource->m_ColCount + m_Resource->m_ColKeys.at(key)] };
			}

			String operator[](const char* key)
			{
				return { m_Resource->m_Data[m_RowNum * m_Resource->m_ColCount + m_Resource->m_ColKeys.at(key)] };
			}

		};

		// ヘッダ（1行目）を除外したループ用のインタフェース
		class WithoutHeaderView {
			friend CSVResource;
		private:
			CSVResource* m_Resource;
			WithoutHeaderView(CSVResource* resource) : m_Resource(resource) {}
			
		public:
			auto begin()
			{
				if (m_Resource->GetRowCount() > 0) {
					return m_Resource->m_RowViews.begin() + 1;
				}
				return m_Resource->m_RowViews.begin();
			}

			auto end()
			{
				return m_Resource->m_RowViews.end();
			}
		};

		static std::vector<std::string> ParseCSVLine(const std::string& line);

	private:
		std::vector<std::string> m_Data;
		std::vector<RowView> m_RowViews;
		size_t m_ColCount;
		std::map<std::string, unsigned int> m_RowKeys;
		std::map<std::string, unsigned int> m_ColKeys;

		void Parse(std::istream& stream);
	public:
		CSVResource(const char* filename);
		CSVResource(const unsigned char* data, size_t size);

		size_t GetRowCount() const { return m_RowViews.size(); }
		size_t GetColCount() const { return m_ColCount; }
		WithoutHeaderView WithoutHeader() { return WithoutHeaderView{ this }; }
		auto begin() { return m_RowViews.begin(); }
		auto end() { return m_RowViews.end(); }
		RowView& operator[](const int i) { return m_RowViews[i]; }
		RowView& operator[](std::string key) { return m_RowViews[m_RowKeys.at(key)]; }
		RowView& operator[](const char* key) { return m_RowViews[m_RowKeys.at(key)]; }

	};

} // namespace MG

