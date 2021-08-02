#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <map>
#include <iomanip>
#include <chrono>
#include <bitset>

char binToChar(const std::string& bin)
{
	char c = 0;
	for (auto b : bin) (c <<= 1) += (b == '1');
	return c;
}

std::wstring readFile(std::string filename)
{
	std::wifstream file(filename);
	std::wstring content;
	for (std::wstring line; getline(file, line); content += line += '\n');
	return content;
}

std::vector<std::pair<int, wchar_t>> countUsage(const std::wstring& text)
{
	// usage counting by map
	std::map<wchar_t, int> m;
	for (wchar_t c : text) m[c]++;

	// turn map into vector
	std::vector<std::pair<int, wchar_t>> v;
	for (auto [c, i] : m) v.emplace_back(i, c);

	// sort characters by num. of usage and return
	std::sort(v.begin(), v.end());
	return v;
}

std::unordered_map<wchar_t, std::string> makeCodeMap(const std::vector<std::pair<int, wchar_t>>& chars)
{
	std::vector<std::pair<int, std::wstring>> v;
	for (auto [i, c] : chars) v.emplace_back(i, std::wstring{ c });

	std::unordered_map<wchar_t, std::string> code;

	while (v.size() > 1)
	{
		auto t0 = v.back();
		v.pop_back();
		for (auto c : t0.second) code[c] = '0' + code[c];

		auto t1 = v.back();
		v.pop_back();
		for (auto i : t1.second) code[i] = '1' + code[i];

		v.emplace_back(t0.first + t1.first, t0.second + t1.second);
		std::sort(v.rbegin(), v.rend());
	}

	return code;
}

void saveBinary(const std::string& msg, std::string filename)
{
	std::ofstream out(filename, std::ofstream::binary);
	for (auto i = 0; i < msg.length(); i += 8)
		out << binToChar(msg.substr(i, 8));
}

void encode(std::string filePlain, std::string fileCode, std::string fileEncoded)
{
	// read plain messege from file
	auto text = readFile(filePlain);

	// count characters
	auto chars = countUsage(text);

	// make map based on text by Huffman alg. (thank you std::unordered_map)
	auto m = makeCodeMap(chars);

	// messege encoding
	std::string msg; // string of binary codes
	for (wchar_t c : text) msg += m.at(c);

	// normalize length of messege
	int a = (8 - msg.length() % 8) % 8;
	msg += std::string(a, '0');

	// binary save to file
	saveBinary(msg, fileEncoded);

	std::cout << "Size of compressed file is : " << msg.size() / 8 << "\n";

	// save code map and metadata to file
	std::ofstream code(fileCode);
	code << a << "\n";
	for (auto [i, c] : chars)
		code << m.at(c) << " " << int(c) << "\n";
}

void decode(std::string fileEncoded, std::string fileCode, std::string fileDecoded)
{
	// read code map from file
	std::unordered_map<std::string, wchar_t> code;
	std::ifstream in(fileCode);
	int a;
	in >> a;
	for (std::string key, value; in >> key >> value; code[key] = stoi(value)); // read code map

	// read encoded messege text from file
	std::string binMsg;
	std::wstring msg;
	std::ifstream in2(fileEncoded, std::ifstream::binary);
	for (char c; in2.get(c); binMsg += std::bitset<8>(c).to_string());

	// decode messege text by code map
	std::wofstream out(fileDecoded);
	binMsg.erase(binMsg.length() - a);
	std::string bin = "";
	for (auto b : binMsg) // decode messege by catterpilar alg.
	{
		bin += b;
		if (code[bin])
		{
			out << code.at(bin);
			bin = "";
		}
	}
}

int main(int argc, char** argv)
{
	auto Start = std::chrono::high_resolution_clock::now();

	if (std::string(argv[1]) == std::string("-c")) encode(argv[2], argv[3], argv[4]);
	else if (std::string(argv[1]) == std::string("-d")) decode(argv[2], argv[3], argv[4]);
	else std::cerr << "unkown command\n";

	auto End = std::chrono::high_resolution_clock::now();

	double time_passed = (1e-9)*std::chrono::duration_cast<std::chrono::nanoseconds>(End - Start).count();
	std::cout << "Timne passed: " << std::fixed << time_passed << std::setprecision(5) << " sec\n";
	return 0;
}
