#include "stdafx.h"
#include "iostream"
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <time.h>
using namespace std;

const int Threshold = 4;
time_t TBegin1, TEnd1;
time_t TBegin2, TEnd2;

void AddTrainingCount(vector<map<string, int>> *Doc, string str, int t)
{
	map<string, int>::iterator iter;
	iter = Doc->at(t - 1).find(str);
	if (iter == Doc->at(t - 1).end()) {
		Doc->at(t - 1).insert(make_pair(str, 1));
	}
	else {
		Doc->at(t - 1)[str]++;
	}
}

void AddTestCount(map<string, int> *TDoc, string Tstr) {
	map<string, int>::iterator iter;
	iter = TDoc->find(Tstr);
	if (iter == TDoc->end()) {
		TDoc->insert(make_pair(Tstr, 1));
	}
	else {
		TDoc->at(Tstr)++;
	}
}

int Classify(map<string, int> *TDoc, vector<map<string, int>> *DocWordCounter, vector<double> *Prob_Docs, vector<double> *Num_Word) {
	double MaxProb = -9999999999;
	double Prob = 0;
	int Label;
	for (int k = 0; k < 15; k++) {
		map<string, int>::iterator Titer;
		for (Titer = TDoc->begin(); Titer != TDoc->end(); Titer++) {
			map<string, int>::iterator iter;
			iter = DocWordCounter->at(k).find(Titer->first);
			if (iter != DocWordCounter->at(k).end()) {
				Prob += log10((DocWordCounter->at(k)[Titer->first] + 1) / (Num_Word->at(k) + 15));
			}
			else {
				Prob += log10(1 / (Num_Word->at(k) + 15));
			}
		}
		Prob += log10(Prob_Docs->at(k));
		if (Prob > MaxProb) {
			MaxProb = Prob;
			Label = k;
		}
		Prob = 0;
	}
	return Label + 1;
}

double Test(vector<int> *Collection_Label, ifstream *TFile, vector<map<string, int>> *DocWordCounter, vector<double> *Prob_Docs, vector<double> *Num_Word) {
	vector<map<string, int>> *TDocWordCounter = new vector<map<string, int>>;
	map<string, int> *TDoc;
	vector<int> *TLabels = new vector<int>;
	string Tbuffer, Tstr;
	while (getline(*TFile, Tbuffer)) {
		TDoc = new map<string, int>;
		int the_last_space = Tbuffer.rfind(' ');//最后一个空格的序号
		int TLabel = stoi(Tbuffer.substr(the_last_space + 2));
		TLabels->push_back(TLabel);
		Tbuffer = Tbuffer.substr(0, the_last_space);
		while (int i = Tbuffer.find(' ')) {
			if (i == -1)
				break;
			Tstr = Tbuffer.substr(0, i);
			Tbuffer = Tbuffer.substr(i + 1);
			if (Tstr.size() < Threshold)continue;
			AddTestCount(TDoc, Tstr);
		}
		TDocWordCounter->push_back(*TDoc);
		delete TDoc;
	}
	(*TFile).close();

	int Hitted = 0;
	int Total = 0;
	int MaxSize = TDocWordCounter->size();
	for (int i = 0; i < MaxSize; i++) {
		int Label = Classify(&TDocWordCounter->at(i), DocWordCounter, Prob_Docs, Num_Word);
		Total++;
		Collection_Label->push_back(Label);
		if (Label == TLabels->at(i))
			Hitted++;
	}
	double Accuracy = (double)Hitted / Total;

	TDocWordCounter->clear();
	delete TDocWordCounter;
	delete TLabels;
	return Accuracy;
}

int main()
{
	TBegin1 = time(NULL);
	vector<map<string, int>> *DocWordCounter = new vector<map<string, int>>(15);
	vector<int> *DocNumCounter = new vector<int>(15);
	vector<double> *WordNumCounter = new vector<double>(15);
	vector<double> *Prob_Docs = new vector<double>(15);//记录每个作者出现的概率
	vector<double> *Num_Word = new vector<double>(15);//记录每个作者的总字数
	double Num_Words = 0;//总字数

	ifstream File("training.txt", ios::in);
	string str;
	string buffer;
	int DocCount = 0;
	while(getline(File, buffer))
	{
		DocCount++;
		int the_last_space = buffer.rfind(' ');//最后一个空格的序号
		int order = stoi(buffer.substr(the_last_space + 2));
		DocNumCounter->at(order - 1)++;
		buffer =buffer.substr(0, the_last_space);
		while (int i = buffer.find(' '))
		{
			if (i == -1)
				break;
			str = buffer.substr(0, i);
			buffer = buffer.substr(i + 1);
			if (str.size() < Threshold)continue;
			Num_Words++;//总字数增加
			Num_Word->at(order - 1)++;//对应作者词数增加
			AddTrainingCount(DocWordCounter, str, order);
			WordNumCounter->at(order - 1)++;
		}
	}
	for (int i = 0; i < 15; i++) {
		Prob_Docs->at(i) = Num_Word->at(i) / Num_Words;
	}
	File.close();

	ifstream *TrainFile = new ifstream;
	TrainFile->open("training.txt", ios::in);
	vector<int> *Collection_Train_Label = new vector<int>;
	double Train_Accuracy = Test(Collection_Train_Label, TrainFile, DocWordCounter, Prob_Docs, Num_Word);
	TEnd1 = time(NULL);

	TBegin2 = time(NULL);
	ifstream *TFile = new ifstream;
	TFile->open("testing.txt", ios::in);
	vector<int> *Collection_Test_Label = new vector<int>;
	double Test_Accuracy = Test(Collection_Test_Label, TFile, DocWordCounter, Prob_Docs, Num_Word);
	TEnd2 = time(NULL);
	
	int Size_Test = Collection_Test_Label->size();
	for (int i = 0; i < Size_Test; i++) {
		cout << Collection_Test_Label->at(i) << endl;
	}
	cout << TEnd1 - TBegin1 << " Seconds (training)" << TEnd1 - TBegin1 << endl;
	cout << TEnd2 - TBegin2 << " Seconds (labeling)" << TEnd2 - TBegin2 << endl;
	cout << Train_Accuracy << " (training)" << endl;
	cout << Test_Accuracy << " (testing)" << endl;
	system("pause");
    return 0;
}