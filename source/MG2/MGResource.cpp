#include "MGResource.h"
#include <fstream>
#include <vector>
#include <windows.h>

namespace MG {

	MGResource::MGResource(const char* filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		
		if (file) {
			size_t size = static_cast<size_t>(file.tellg());
			file.seekg(0, std::ios::beg);

			// ヘッダを読み込む
			MG_RESOURCE_HEADER header;
			file.read(reinterpret_cast<char*>(&header), sizeof(MG_RESOURCE_HEADER));

			// エントリーを読み込む
			std::vector<MG_RESOURCE_ENTRY> entries;
			entries.reserve(header.entryCount);
			for (unsigned int i = 0; i < header.entryCount; i++) {
				MG_RESOURCE_ENTRY entry{};
				file.read(reinterpret_cast<char*>(&entry), sizeof(MG_RESOURCE_ENTRY));
				entries.push_back(entry);
			}

			// ファイルのデータを読み込む
			for (auto entry : entries) {
				ResourceFile resfile{
					new unsigned char[entry.size],
					entry.size
				};
				file.read(reinterpret_cast<char*>(resfile.data), entry.size);
				m_Files[entry.name] = resfile;
			}

			file.close();

		}
	}

	void MGResource::Add(const char* filename, const char* rename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (file.is_open()) {
			size_t size = static_cast<size_t>(file.tellg());
			file.seekg(0, std::ios::beg);
			file.clear();

			ResourceFile resFile{
				new unsigned char[size],
				size
			};

			file.read(reinterpret_cast<char*>(resFile.data), size);

			std::string name = filename;
			if (rename) {
				name = rename;
			}

			// 同じ名前があった場合、上書きする
			if (m_Files.count(name) > 0) {
				delete[] m_Files[name].data;
				m_Files[name].size = 0;
			}

			m_Files[name] = resFile;
			
			file.close();
		}
	}

	void MGResource::Remove(const char* filename)
	{
		if (m_Files.count(filename) > 0) {
			delete[] m_Files[filename].data;
			m_Files.erase(filename);
		}
	}

	MGResource::ResourceFile MGResource::GetFile(const char* filename)
	{
		if (m_Files.count(filename) > 0) {
			return m_Files[filename];
		}
		return {};
	}

	void MGResource::Write(const char* filename)
	{

		// ヘッダ作成
		MG_RESOURCE_HEADER header{};
		strcpy_s(header.version, sizeof(char) * ARRAYSIZE(header.version), VERSION);
		header.entryCount = static_cast<unsigned int>(m_Files.size());

		// エントリー作成
		std::vector<MG_RESOURCE_ENTRY> entries;
		entries.reserve(header.entryCount);
		for (auto pair : m_Files) {
			MG_RESOURCE_ENTRY entry{};
			strcpy_s(entry.name, sizeof(char) * ARRAYSIZE(entry.name), pair.first.c_str());
			entry.size = pair.second.size;
			entries.push_back(entry);
		}

		// 出力
		std::ofstream file(filename, std::ios::binary);
		file.write((const char*)&header, sizeof(MG_RESOURCE_HEADER));
		file.write((const char*)entries.data(), sizeof(MG_RESOURCE_ENTRY) * entries.size());
		for (auto pair : m_Files) {
			file.write((const char*)pair.second.data, pair.second.size);
		}

		file.close();
	}

	void MGResource::Release()
	{
		for (auto pair : m_Files) {
			auto file = pair.second;
			delete[] file.data;
		}
		m_Files.clear();
	}
	

} // namespace MG