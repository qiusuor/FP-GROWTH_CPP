#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <string>
#include <functional>
#include <sstream>
#include <ctime>

//using namespace std;

class fp_growth {
private:
	typedef struct Data
	{
		std::vector<std::string> info;
		int sup;
		Data*next;
	}*Data_ptr;//���������Ԥ�����ļ����ݵģ������е�������������ʽ�������ڴ��У��������ʹ�ã����԰������ļ�ɨ��ת��Ϊһ���ļ�������ڴ������Ψһ�Ĵ����������ڴ�����

	struct node
	{
		std::string name;
		int sup;
		node* link;
		node* parent;
		std::map<std::string, node*> children;
	};//�ֵ����ڵ�

	struct head
	{
		node* link;
		int sup;
	};//���������Ǳ���ÿ��������ͬ�Ľڵ����ֵ����е�λ��  map<string,head>    string ��ʾ��Ʒ����
	struct sort_help
	{
		std::string name;
		int sup;
	};//��һ�����׼�¼�еĸ��������֧�ֶ�����


	struct result_fp
	{
		std::map<std::string, head> header;
		node* root;
	};//��fp-growth �����㷨�Ĳ�����������Ҫ�������Ȩֵ���ֵ���������ÿ�����ֽڵ����ֵ�����λ�õ�ӳ�䣩�������
	
	struct outcome {
		std::vector<std::string>items;
		int sup;
		double confidence_level;
		bool operator< (const outcome &b) const
		{
			return this->items < b.items;
		}
	};//������
	std::string file_name;//���ݼ�������  *.csv
	int minsup;
	double confidence;
public:
	fp_growth(std::string file_name, int minsup,double confidence) :file_name(file_name), minsup(minsup) ,confidence(confidence){ }
	void fp();//Ψһ�Ķ���ӿ�
	~fp_growth() { file_name.clear(); }
protected:
	static bool fp_growth::cmp(sort_help& a, sort_help& b) { return a.sup > b.sup; }
	int read_help(std::string &pline);
	//void calculate_confidece(std::set<outcome>& out);
	void read_help_1(std::string &pline, std::string &line);
	Data_ptr preconditioning(std::string&file_name);
	result_fp* creat_fp(Data_ptr& data, int minsup = 10);
	Data_ptr find_prefix(node* first);
	void fp_growth::write_result(std::set<outcome>& out);
	void fp_growth::mine_tree(result_fp *result, int minsup, outcome &pre, std::set<outcome> &out);
};

int fp_growth::read_help(std::string &pline) {
	for (int i = 5;; i++)
	{
		if (pline[i] == '{')
		{
			return i;
		}
	}
}
void fp_growth::read_help_1(std::string &pline, std::string &line) {
	line.clear();
	int start = read_help(pline) + 1;
	for (size_t i = start; i < pline.size() - 2; i++)
	{
		line += pline[i];
	}
}
fp_growth::Data_ptr fp_growth::preconditioning(std::string&file_name) {
	std::ifstream in_file(file_name);
	std::stringstream buffer;
	buffer << in_file.rdbuf();
	Data_ptr data = new Data;//�׽ڵ����
	Data_ptr cur = data;
	Data_ptr te;
	std::string line;
	unsigned int begin;
	unsigned int end;
	
	std::string pline;
	getline(buffer, pline);//�����в���������������
	//int count = 0;
	for (getline(buffer, pline); pline != ""; getline(buffer, pline))
	{
		read_help_1(pline, line);
		line = line + ',';
		te = new Data;
		te->next = NULL;
		for (begin = 0, end = 0; end < line.size(); end++)
		{
			if (line[end] == ',') {
				/*if (line.substr(begin, end - begin)=="bottled water")
				{
					count++;
				}*/
				te->info.push_back(line.substr(begin, end - begin));
				begin = end + 1;
			}
		}
		te->sup = 1;
		cur->next = te;
		cur = te;
	}
	//std::cout << count;
	in_file.close();
	return data;
}

fp_growth::result_fp* fp_growth::creat_fp(fp_growth::Data_ptr& data, int minsup) {
	
	node temp_node;

	Data_ptr cur = data->next;//data���׽ڵ��ǿյ�
	std::map<std::string, head> header;
	while (cur) {
		for (int i = 0; i < cur->info.size(); i++)
		{
			if (header.count(cur->info[i]))
				header[cur->info[i]].sup += cur->sup;
			else {
				header[cur->info[i]].sup = cur->sup;
				header[cur->info[i]].link = NULL;
			}
		}
		cur = cur->next;
	}
	//header = header;
	//data->next = data->next;
	for (std::map<std::string, head>::iterator i = header.begin(); i != header.end();)
	{
		if (i->second.sup < minsup)
		{
			header.erase(i++);
		}
		else i++;
	}
	//header = header;
	
	node* root = new node;
	root->parent = NULL;
	root->link = NULL;
	root->name = "";
	root->sup = 0;
	root->children.clear();

	node* cur_tree = NULL;
	node* new_child = NULL;
	node* cur_head;

	std::vector<sort_help> item;//һ�����׼�¼
	sort_help sort_help_temp;
	cur = data->next;


	while (cur) {
		for (int i = 0; i < cur->info.size(); i++)
		{

			if (header.count(cur->info[i]))
			{
				sort_help_temp.sup = header[cur->info[i]].sup;
				sort_help_temp.name = cur->info[i];
				item.push_back(sort_help_temp);
			}
		}
		if (item.size()) {
			sort(item.begin(), item.end(), fp_growth::cmp);
			cur_tree = root;

			for (int i = 0; i < item.size(); i++)
			{
				cur_head = header[item[i].name].link;//���Ϊnull����˵��header û����Ӧnode�ĺ��
				//while (cur_head&&cur_head->link) {
				//	cur_head = cur_head->link;
				//}//��Ϊ�գ������Ƶ����һ��node��λ��
				if (cur_tree->children.count(item[i].name))
				{
					cur_tree->children[item[i].name]->sup += cur->sup;
					cur_tree = cur_tree->children[item[i].name];
				}
				else {
					new_child = new node;
					new_child->name = item[i].name;
					new_child->sup = cur->sup;
					new_child->children.clear();
					new_child->link = NULL;
					new_child->parent = cur_tree;
					cur_tree->children[item[i].name] = new_child;
					cur_tree = new_child;
					header[item[i].name].link = new_child;
					new_child->link = cur_head;
					/*if (cur_head == NULL)
					{
						header[item[i].name].link = new_child;
					}
					else {
						cur_head->link = new_child;
					}*/
				}
			}
			//���뵽���в�ά��head
			item.clear();
		}
		cur = cur->next;
	}
	result_fp *result = new result_fp;
	result->header = header;
	result->root = root;

	return result;
}
fp_growth::Data_ptr fp_growth::find_prefix(fp_growth::node* first) {
	Data_ptr data = new Data;
	Data_ptr te;
	Data_ptr cur_data = data;
	data->next = NULL;
	data->sup = 0;
	node* cur_node = first;
	node* ff;

	while (cur_node != NULL) {
		if (cur_node->parent->parent != NULL)
		{
			te = new Data;
			te->sup = cur_node->sup;
			te->next = NULL;
			ff = cur_node;
			while (ff->parent->name != "") {
				te->info.push_back(ff->parent->name);
				ff = ff->parent;
			}
			cur_data->next = te;
			cur_data = te;
		}
		cur_node = cur_node->link;
	}
	return data;
} //���û��ǰ׺�����ؿյĽڵ�

void fp_growth::mine_tree(result_fp *result, int minsup, outcome &pre, std::set<outcome> &out) {
	std::map<std::string, head> header = result->header;
	node *root = result->root;
	for (auto it : header )
	{
		pre.items.push_back(it.first);
		pre.sup = it.second.sup;
		out.insert(pre);
		if (it.second.link != NULL)
		{
			Data_ptr ll = find_prefix(it.second.link);
			result_fp *f = creat_fp(ll, minsup);
			if (f->header.size())
				mine_tree(f, minsup, pre, out);
		}
		pre.items.pop_back();
		pre.sup = 0;
	}
}
void fp_growth:: write_result(std::set<outcome>& out) {
	std::vector<std::string> refer;
	std::vector<std::string> copy_of_refer;
	std::map<std::vector<std::string>, int> items_times;
	double con;

	int i = 0;
	unsigned int max_times=0;
	std::ofstream out_file("C:\\rule.csv");
	out_file << "\"\",\"Item\",\"Suport\",\"Confidence\"\n";
	for (auto it:out)
		items_times[it.items] = it.sup;
	for (auto it : out)
	{
		if (it.items.size() == 1)
			con = 1;
		else {
			copy_of_refer = it.items;
			for (int i = 0; i < copy_of_refer.size(); i++) {
				for (int j=0;j<copy_of_refer.size();j++)
				{
					if (i != j)
						refer.push_back(copy_of_refer[j]);
				}
				refer == refer;
				if (items_times.count(refer)) {
					if (items_times[refer] > max_times)
						max_times = items_times[refer];
				}	
				else {
					max_times = 99999999;
					refer.clear();
					break;
				}
				refer.clear();
			}
			con = double(items_times[copy_of_refer])/max_times ;
		}
		if (con>confidence)
		{
			out_file << "\"" << ++i << "\",\"{";
			for (std::vector<std::string> ::iterator i = it.items.begin(); i != it.items.end(); i++)
			{
				out_file << *i;
				if (i + 1 != it.items.end())
				{
					out_file << ",";
				}
			}
			out_file << "}\"," << "\"" << it.sup << "\",\"" << con << "\"";
			out_file << std::endl;
		}
	}
	out_file.close();
}

void fp_growth::fp() {
	Data_ptr data = preconditioning(file_name);
	result_fp * result = creat_fp(data, minsup);
	outcome pre;
	std::set<outcome> out;
	mine_tree(result, minsup, pre, out);
	std::set<outcome> odered_out;
	outcome one_rule;
	for (auto i : out)
	{
		one_rule = i;
		sort(one_rule.items.begin(), one_rule.items.end(), std::less<std::string>());
		odered_out.insert(one_rule);
	}
	/*if (choice)
		calculate_confidece(out);*/
	write_result(odered_out);
}


//void fp_growth::calculate_confidece(std::set<outcome>& out) {
//	std::vector<std::string> refer;
//	std::map<std::vector<std::string>, int> items_times;
//
//	for (auto it : out)
//	{
//		items_times[it.items] = it.sup;
//		if (it.items.size() == 1)
//			it.confidence_level = 1;
//		else {
//			refer =	it.items;
//			refer.pop_back();
//			it.confidence_level = (double)it.sup / items_times[refer];
//		}
//	}
//}
