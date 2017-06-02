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
	}*Data_ptr;//这个是用来预处理文件数据的，把所有的数据以这样格式保存在内存中，方便后面使用；可以把两次文件扫描转化为一次文件缓冲和内存操作，唯一的代价是增大内存需求

	struct node
	{
		std::string name;
		int sup;
		node* link;
		node* parent;
		std::map<std::string, node*> children;
	};//字典树节点

	struct head
	{
		node* link;
		int sup;
	};//它的作用是保存每个名字相同的节点在字典树中的位置  map<string,head>    string 表示商品名字
	struct sort_help
	{
		std::string name;
		int sup;
	};//将一条交易记录中的各个项按照总支持度排序


	struct result_fp
	{
		std::map<std::string, head> header;
		node* root;
	};//把fp-growth 建树算法的产生的两个必要结果（带权值的字典树，保存每个出现节点在字典树中位置的映射）打包传递
	
	struct outcome {
		std::vector<std::string>items;
		int sup;
		double confidence_level;
		bool operator< (const outcome &b) const
		{
			return this->items < b.items;
		}
	};//保存结果
	std::string file_name;//数据集的名字  *.csv
	int minsup;
	double confidence;
public:
	fp_growth(std::string file_name, int minsup,double confidence) :file_name(file_name), minsup(minsup) ,confidence(confidence){ }
	void fp();//唯一的对外接口
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
	Data_ptr data = new Data;//首节点设空
	Data_ptr cur = data;
	Data_ptr te;
	std::string line;
	unsigned int begin;
	unsigned int end;
	
	std::string pline;
	getline(buffer, pline);//读首行并丢弃，首行无用
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

	Data_ptr cur = data->next;//data的首节点是空的
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

	std::vector<sort_help> item;//一条交易记录
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
				cur_head = header[item[i].name].link;//如果为null，则说明header 没有相应node的后继
				//while (cur_head&&cur_head->link) {
				//	cur_head = cur_head->link;
				//}//不为空，将其推到最后一个node的位置
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
			//插入到树中并维护head
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
} //如果没有前缀，返回空的节点

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
