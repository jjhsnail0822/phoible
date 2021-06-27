#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <ctime>
#include <thread>
#include <future>
using namespace std;

#define NUM_OF_COM_FOR_SORT 9 //9
#define VIEW_ROW_NUM 5
#define THREAD_NUM 16
#define MIN_APPEAR 30
#define MEMORY_BYTE 8192*1024*1024 //8192
#define MEMORY_DIVIDE_C 32

class Phoible {
private:
	vector<string> data_langName;
	// data_c,v,t only have allophones
	vector<vector<vector<string>>> data_consonants;
	vector<vector<vector<string>>> data_vowels;
	vector<vector<vector<string>>> data_tones;
	vector<string> all_consonants;
	vector<string> all_vowels;
	vector<string> all_tones;
	vector<int> all_consonants_num;
	vector<int> all_vowels_num;
	vector<int> all_tones_num;
	vector<int> com_lang_num_list;
	int thread_num = THREAD_NUM;
	string bitmask_manyComb;
	long long int N_manyComb, K_manyComb;
	vector<string> allList;

	vector<vector<string>> comlist_manyComb;
	long long int manyComb_index;

public:
	vector<vector<string>> comlist;

	Phoible(string path="phoible.csv")
	{
		vector<vector<string>> csvData;

		ifstream file(path);
		if (file.fail())
		{
			cout << "file not exist" << endl;
			throw;
		}
		
		vector<string> row = csv_read_row(file, ','); // remove titles
		while (file.good())
		{
			row = csv_read_row(file, ',');
			csvData.push_back(row);
		}
		file.close();

		// spa 1979 world 197
		// aa 1993 africa 200
		// ph 2014 world
		// gm 2014 africa
		// uz 2014

		string lang_name = "";
		int lang_index = -1;
		string nowLangName;
		vector<vector<string>>::iterator row_iter;
		for (row_iter = csvData.begin(); row_iter != csvData.end(); row_iter++)
		{
			//if ((*row_iter)[10] != "spa" && (*row_iter)[10] != "ph" && (*row_iter)[10] != "uz") // Source
			if ((*row_iter)[10] != "spa") // Source
				continue;

			nowLangName = (*row_iter)[3];
			transform(nowLangName.begin(), nowLangName.end(), nowLangName.begin(), tolower);
			if (nowLangName != lang_name)
			{
				lang_name = nowLangName;
				data_langName.push_back(lang_name);
				vector<vector<string>> langData;
				data_consonants.push_back(langData);
				data_vowels.push_back(langData);
				data_tones.push_back(langData);
				lang_index++;
			}
			vector<string> allophonesData;
			stringstream ss((*row_iter)[7]);
			string allophone;
			while (getline(ss, allophone, ' '))
			{
				allophonesData.push_back(allophone);
			}
			bool isFound = false;
			int j = 0;
			if ((*row_iter)[9] == "consonant")
			{
				data_consonants[lang_index].push_back(allophonesData);
				for (vector<string>::iterator iter = all_consonants.begin(); iter != all_consonants.end(); iter++)
				{
					if ((*iter) == (*row_iter)[6])
					{
						isFound = true;
						break;
					}
					j++;
				}
				if (isFound)
				{
					all_consonants_num[j]++;
				}
				else
				{
					all_consonants.push_back((*row_iter)[6]);
					all_consonants_num.push_back(1);
				}
			}
			else if ((*row_iter)[9] == "vowel")
			{
				data_vowels[lang_index].push_back(allophonesData);
				for (vector<string>::iterator iter = all_vowels.begin(); iter != all_vowels.end(); iter++)
				{
					if ((*iter) == (*row_iter)[6])
					{
						isFound = true;
						break;
					}
					j++;
				}
				if (isFound)
				{
					all_vowels_num[j]++;
				}
				else
				{
					all_vowels.push_back((*row_iter)[6]);
					all_vowels_num.push_back(1);
				}
			}
			else if ((*row_iter)[9] == "tone")
			{
				data_tones[lang_index].push_back(allophonesData);
				for (vector<string>::iterator iter = all_tones.begin(); iter != all_tones.end(); iter++)
				{
					if ((*iter) == (*row_iter)[6])
					{
						isFound = true;
						break;
					}
					j++;
				}
				if (isFound)
				{
					all_tones_num[j]++;
				}
				else
				{
					all_tones.push_back((*row_iter)[6]);
					all_tones_num.push_back(1);
				}
			}
			else
			{
				cout << "corupted file error" << endl;
			}
		}		
	}

	~Phoible()
	{

	}
	
	// http://www.zedwood.com/article/cpp-csv-parser
	std::vector<std::string> csv_read_row(std::istream& in, char delimiter)
	{
		std::stringstream ss;
		bool inquotes = false;
		std::vector<std::string> row;//relying on RVO
		while (in.good())
		{
			char c = in.get();
			if (!inquotes && c == '"') //beginquotechar
			{
				inquotes = true;
			}
			else if (inquotes && c == '"') //quotechar
			{
				if (in.peek() == '"')//2 consecutive quotes resolve to 1
				{
					ss << (char)in.get();
				}
				else //endquotechar
				{
					inquotes = false;
				}
			}
			else if (!inquotes && c == delimiter) //end of field
			{
				row.push_back(ss.str());
				ss.str("");
			}
			else if (!inquotes && (c == '\r' || c == '\n'))
			{
				if (in.peek() == '\n') { in.get(); }
				row.push_back(ss.str());
				return row;
			}
			else
			{
				ss << c;
			}
		}
	}

	void comb(int K)
	{
		int N = allList.size();

		std::string bitmask(K, 1); // K leading 1's
		bitmask.resize(N, 0); // N-K trailing 0's

		// print integers and permute bitmask
		do {
			vector<string> com;
			for (int i = 0; i < N; ++i) // [0..N-1] integers
			{
				if (bitmask[i])
				{
					com.push_back(allList[i]);
				}
			}
			comlist.push_back(com);
		} while (std::prev_permutation(bitmask.begin(), bitmask.end()));
	}

	void manyComb_init(int K_init, long long int size)
	{
		N_manyComb = allList.size();
		K_manyComb = K_init;
		string bitmask_init(K_manyComb, 1);
		bitmask_manyComb = bitmask_init;
		bitmask_manyComb.resize(N_manyComb, 0);
		manyComb_index = 0;

		vector<string> com;
		for (int i = 0; i < N_manyComb; ++i)
		{
			if (bitmask_manyComb[i])
			{
				com.push_back(allList[i]);
			}
		}
		comlist.push_back(com);
		
		long long int size_i = 1;
		while (std::prev_permutation(bitmask_manyComb.begin(), bitmask_manyComb.end()))
		{
			vector<string> com_;
			for (int i = 0; i < N_manyComb; ++i) // [0..N-1] integers
			{
				if (bitmask_manyComb[i])
				{
					com_.push_back(allList[i]);
				}
			}
			comlist.push_back(com_);

			++size_i;
			if (size_i >= size)
				break;
		}
	}

	bool manyComb_iter(long long int size)
	{
		++manyComb_index;
		printf("\nMaking new combination part %lld...\n", manyComb_index);
		comlist.clear();
		long long int size_i = 0;
		bool partBreak = false;

		while (std::prev_permutation(bitmask_manyComb.begin(), bitmask_manyComb.end()))
		{
			vector<string> com;
			for (int i = 0; i < N_manyComb; ++i) // [0..N-1] integers
			{
				if (bitmask_manyComb[i])
				{
					com.push_back(allList[i]);
				}
			}
			comlist.push_back(com);

			++size_i;
			if (size_i >= size)
			{
				partBreak = true;
				break;
			}
		}
		if (partBreak)
			return true;
		else
			return false;
	}

	void phoneme_combinations(string type, int num, int min_appear, long long int combination_part_size)
	{
		allList.clear();
		int j = 0;
		if (type == "consonant")
		{
			for (auto iter = all_consonants.begin(); iter != all_consonants.end(); iter++, j++)
			{
				if (all_consonants_num[j] >= min_appear)
					allList.push_back(*iter);
			}
		}
		else if (type == "vowel")
		{
			for (auto iter = all_vowels.begin(); iter != all_vowels.end(); iter++, j++)
			{
				if (all_vowels_num[j] >= min_appear)
					allList.push_back(*iter);
			}
		}
		else if (type == "tone")
		{
			for (auto iter = all_tones.begin(); iter != all_tones.end(); iter++, j++)
			{
				if (all_tones_num[j] >= min_appear)
					allList.push_back(*iter);
			}
		}
		else
		{
			cout << "combination type error" << endl;
			throw;
		}
		comlist.clear();

		clock_t start, end;
		double result;
		start = clock();
		
		if (combination_part_size != 0)
		{
			printf("Initializing combination & time checking...\n");
			manyComb_init(num, combination_part_size);
		}
		else
		{
			cout << "Start combination & time checking..." << endl;
			comb(num);
		}

		end = clock();
		result = (double)(end - start) / 1000;
		cout << result << "s" << endl;
	}

	static bool compare_comlist(vector<string>& A, vector<string>& B)
	{
		return stoi(A.back()) > stoi(B.back());
	}

	void choose_max_coms()
	{
		vector<vector<string>> view_data;
		for (int view_row = 0; view_row < VIEW_ROW_NUM; ++view_row)
		{
			int max = 0;
			long long int comlist_i = 0;
			long long int comlist_i_max = 0;
			vector<int>::iterator max_iter;
			for (auto i = com_lang_num_list.begin(); i != com_lang_num_list.end(); ++i)
			{
				int this_count = *i;
				if (max <= this_count)
				{
					max = this_count;
					max_iter = i;
					comlist_i_max = comlist_i;
				}
				++comlist_i;
			}
			vector<string> this_view_data(comlist[comlist_i_max]);
			this_view_data.push_back(to_string(*max_iter));
			com_lang_num_list.erase(max_iter);
			comlist.erase(comlist.begin() + comlist_i_max);
			view_data.push_back(this_view_data);
		}
		comlist = view_data;
	}

	void run_thread(long long int start_i, long long int end_i, vector<vector<vector<string>>>* pData, vector<string>* all_list_pointer, vector<vector<int>>* com_num, vector<vector<string>>* local_comlist, promise<vector<int>>&& thread_result)
	{
		vector<int> com_lang_num_list_thread;

		int com_lang_num = 0;
		bool isComFound = false;
		bool isComExist = false;
		int** phoneme_check_p = new int* [(*pData).size()];
		for (int i = 0; i < pData->size(); ++i)
		{
			phoneme_check_p[i] = new int[(*all_list_pointer).size()];
			memset(phoneme_check_p[i], 0xFF, sizeof(int) * (all_list_pointer->size())); // set to -1
		}

		int data_size = (*pData).size();
		int com_size = (*local_comlist)[0].size();
		vector<int> com_check;
		int phoneme_check_this;

		long long int goal = 0;
		long long int goal_add = (end_i - start_i) / 100;
		for (long long int i = start_i; i < end_i; ++i) // com loop
		{
			if (i == goal)
			{
				printf("#");
				goal += goal_add;
			}
			com_lang_num = 0;
			for (int j = 0; j < data_size; ++j) // lang loop
			{
				com_check.clear();
				for (int k = 0; k < com_size; ++k) // com_phoneme loop
				{
					isComFound = false;
					isComExist = false;
					phoneme_check_this = phoneme_check_p[j][(*com_num)[i][k]];
					if (phoneme_check_this == -2)
						break;
					else if (phoneme_check_this != -1)
					{
						isComExist = true;
						if (find(com_check.begin(), com_check.end(), phoneme_check_this) != com_check.end()) // if found
							break;
						else
						{
							isComFound = true;
							com_check.push_back(phoneme_check_this);
						}
					}
					for (auto l = 0; l < (*pData)[j].size(); ++l) // phoneme loop
					{
						if (find((*pData)[j][l].begin(), (*pData)[j][l].end(), (*local_comlist)[i][k]) != (*pData)[j][l].end()) // if found
						{
							isComExist = true;
							if (find(com_check.begin(), com_check.end(), l) != com_check.end())
								break;
							else
							{
								isComFound = true;
								phoneme_check_p[j][(*com_num)[i][k]] = l;
								com_check.push_back(l);
								break;
							}
						}
					}
					if (!isComExist)
					{
						--phoneme_check_p[j][(*com_num)[i][k]];
						break;
					}
					if (!isComFound)
						break;
				}
				if (isComFound)
					++com_lang_num;
			}
			com_lang_num_list_thread.push_back(com_lang_num);
		}

		for (int i = 0; i < (*pData).size(); ++i)
		{
			delete[]phoneme_check_p[i];
		}
		delete[]phoneme_check_p;

		thread_result.set_value(com_lang_num_list_thread);
	}

	void run(string type, int number_of_combination)
	{
		long long int combination_part_size = (long long int)MEMORY_BYTE / number_of_combination / MEMORY_DIVIDE_C;

		vector<string>* all_list_pointer;
		vector<vector<vector<string>>>* pData;
		if (type == "consonant")
		{
			all_list_pointer = &all_consonants;
			pData = &data_consonants;
		}
		else if (type == "vowel")
		{
			all_list_pointer = &all_vowels;
			pData = &data_vowels;
		}
		else if (type == "tone")
		{
			all_list_pointer = &all_tones;
			pData = &data_tones;
		}
		else
		{
			cout << "run type error";
			throw;
		}

		if (number_of_combination < NUM_OF_COM_FOR_SORT)
		{
			phoneme_combinations(type, number_of_combination, MIN_APPEAR, 0);
		}
		else
		{
			phoneme_combinations(type, number_of_combination, MIN_APPEAR, combination_part_size);
		}

		vector<vector<string>>* comlist_p = &comlist;

		clock_t start, end;
		double result;
		cout << "Start running & time checking..." << endl;
		start = clock();

		com_lang_num_list.clear();

		vector<vector<int>>* com_num_p;
		vector<vector<int>> com_num;
		for (auto i = comlist.begin(); i != comlist.end(); ++i)
		{
			vector<int> com_num_temp;
			for (auto j = i->begin(); j != i->end(); ++j)
			{
				com_num_temp.push_back(find(all_list_pointer->begin(), all_list_pointer->end(), (*j)) - all_list_pointer->begin());
			}
			com_num.push_back(com_num_temp);
		}
		com_num_p = &com_num;

		long long int comlist_size = comlist.size();
		vector<future<vector<int>>> f_thread_result;
		vector<thread> workers;
		long long int local_thread_num = (long long int)thread_num;

		printf("Start threads...\n");

		for (long long int t = 0; t < local_thread_num; ++t)
		{
			promise<vector<int>> p_thread_result;
			f_thread_result.push_back(p_thread_result.get_future());
			if (t != local_thread_num - 1)
				workers.push_back(thread(&Phoible::run_thread, this, comlist_size / local_thread_num * t, comlist_size / local_thread_num * (t + 1), pData, all_list_pointer, com_num_p, comlist_p, move(p_thread_result)));
			else
				workers.push_back(thread(&Phoible::run_thread, this, comlist_size / local_thread_num * t, comlist_size, pData, all_list_pointer, com_num_p, comlist_p, move(p_thread_result)));
		}

		for (int t = 0; t < local_thread_num; ++t)
		{
			f_thread_result[t].wait();
			vector<int> result_data = f_thread_result[t].get();
			com_lang_num_list.insert(com_lang_num_list.end(), result_data.begin(), result_data.end());
			workers[t].join();
		}

		if (number_of_combination >= NUM_OF_COM_FOR_SORT)
		{
			choose_max_coms();
			comlist_manyComb.insert(comlist_manyComb.end(), comlist.begin(), comlist.end());
			comlist.clear();

			while (manyComb_iter(combination_part_size))
			{
				comlist_p = &comlist;
				com_lang_num_list.clear();
				com_num.clear();
				for (auto i = comlist.begin(); i != comlist.end(); ++i)
				{
					vector<int> com_num_temp;
					for (auto j = i->begin(); j != i->end(); ++j)
					{
						com_num_temp.push_back(find(all_list_pointer->begin(), all_list_pointer->end(), (*j)) - all_list_pointer->begin());
					}
					com_num.push_back(com_num_temp);
				}
				com_num_p = &com_num;

				long long int comlist_size_manyComb = comlist.size();
				vector<future<vector<int>>> f_thread_result_manyComb;
				vector<thread> workers_manyComb;

				printf("Start part threads...\n");

				for (long long int t = 0; t < local_thread_num; ++t)
				{
					promise<vector<int>> p_thread_result;
					f_thread_result_manyComb.push_back(p_thread_result.get_future());
					if (t != local_thread_num - 1)
						workers_manyComb.push_back(thread(&Phoible::run_thread, this, comlist_size_manyComb / local_thread_num * t, comlist_size_manyComb / local_thread_num * (t + 1), pData, all_list_pointer, com_num_p, comlist_p, move(p_thread_result)));
					else
						workers_manyComb.push_back(thread(&Phoible::run_thread, this, comlist_size_manyComb / local_thread_num * t, comlist_size_manyComb, pData, all_list_pointer, com_num_p, comlist_p, move(p_thread_result)));
				}

				for (int t = 0; t < local_thread_num; ++t)
				{
					f_thread_result_manyComb[t].wait();
					vector<int> result_data = f_thread_result_manyComb[t].get();
					com_lang_num_list.insert(com_lang_num_list.end(), result_data.begin(), result_data.end());
					workers_manyComb[t].join();
				}

				choose_max_coms();
				comlist_manyComb.insert(comlist_manyComb.end(), comlist.begin(), comlist.end());
			}

			comlist.swap(comlist_manyComb);
			vector<vector<string>>().swap(comlist_manyComb);
			vector<int>().swap(com_lang_num_list);

			cout << endl << "Start sorting..." << endl;
			sort(comlist.begin(), comlist.end(), compare_comlist);

			end = clock();
			result = (double)(end - start) / 1000;
			cout << result << "s" << endl;
		}
		else
		{
			cout << endl << "Start sorting..." << endl;

			if (number_of_combination < NUM_OF_COM_FOR_SORT || type == "vowel" || type == "tone")
			{
				long long int num_iter = 0;
				for (auto i = comlist.begin(); i != comlist.end(); ++i)
				{
					i->push_back(to_string(com_lang_num_list[num_iter]));
					++num_iter;
				}
				vector<int>().swap(com_lang_num_list);
				sort(comlist.begin(), comlist.end(), compare_comlist);
			}
			else
			{
				choose_max_coms();
			}

			end = clock();
			result = (double)(end - start) / 1000;
			cout << result << "s" << endl;
		}
	}
};

int main()
{
	locale::global(std::locale("ko_KR.UTF-8"));
	SetConsoleOutputCP(65001); // for Windows terminal

	char input_char;
	int input_num = 1;
	while (input_num != 0)
	{
		Phoible* ph = new Phoible("C:\\Users\\snail\\source\\repos\\Phoible\\Phoible\\phoible.csv");
		cout << ">>";
		cin >> input_char >> input_num;
		if (input_char == 'c')
		{
			ph->run("consonant", input_num);
		}
		else if (input_char == 'v')
		{
			ph->run("vowel", input_num);
		}
		else if (input_char == 't')
		{
			ph->run("tone", input_num);
		}
		else
			continue;
		for (int i = 0; i < VIEW_ROW_NUM; i++)
		{
			for (auto j = ph->comlist[i].begin(); j != ph->comlist[i].end(); j++)
			{
				cout << (*j) << " ";
			}
			cout << endl;
		}
		cout << endl;
		int i = 1;
		bool isLong = false;
		for (auto j=ph->comlist.begin(); j!=ph->comlist.end(); j++)
		{
			if (i > VIEW_ROW_NUM)
				break;
			isLong = false;
			for (auto k = j->begin(); k != (j->end() - 1); k++)
			{
				if (k->size() > 2)
				{
					isLong = true;
					break;
				}
			}
			if (!isLong)
			{
				for (auto k = j->begin(); k != j->end(); k++)
				{
					cout << (*k) << " ";
				}
				cout << endl;
				i++;
			}
		}
		delete ph;
	}
	return 0;
}