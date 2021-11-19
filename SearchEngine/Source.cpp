#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
using namespace std;
struct page {
	string name;
	vector<string> keywords;
	int imp;
	double pageScore;
	int clicks;
	int children = -1;
	double pageRank;
	double CTR;
	static int total;

};
int page::total = 0;
vector<page> created;
multimap<double, page> calcScore(vector<page>[], int, vector<page>);
void calcRank(vector<page>[], int);
void calcCTR(vector<page>[], int);
void readGraph(vector<page> []);
void readKey(vector<page>[]);
void readImp(vector<page>[]);
void addEdge(vector<page> [], int, page);
void addKey(vector<page>[], int, string);
void printGraph(vector<page>[], int);
void printKey(vector<page>[], int);
void printImp(vector<page>[], int);
vector<page> doSearch(vector<page>[], int, string);
vector<string> searchStrAnd(string);
vector<string> searchStr(string);

void main() {
	vector<page> adjlist[10];
	readGraph(adjlist);
	readKey(adjlist);
	readImp(adjlist);						//defining and initializing the adjency matrix representing the graph
	cout << "\nWelcome!\n What would you like to do?\n 1. New search\n 2. Exit\n \n \nType in your choice:   ";
	int choice;
	string search;
	cin >> choice;							//taking input from the user with his choice and search string
	if (choice == 2)
		choice = 3;
	bool calculated = false;
	while (choice != 3) {					//this loop will work until the user chooses to exit
		if (choice != 0) {					//0 means it is returning from a previous search and thus it will not take new search
			cout << "Enter your search:  ";
			cin.ignore();
			getline(cin,search);
		}
		calcCTR(adjlist, page::total);
		if (calculated == false) {
			calcRank(adjlist, page::total);		//calculating CTR each time a search is required and Rank first time only
			calculated = true;
		}
		int pageNo=1;
		multimap<double, page> final= calcScore(adjlist, page::total, doSearch(adjlist, page::total, search));		//getting the map of search results with their scores
		if (final.empty()) {
			cout << "there is no search results\n";
			break;
		}
		cout << "Search results: \n";
		for (auto it = final.rbegin();it != final.rend();it++) {
			cout << pageNo++ << ". " << it->second.name << endl;
		}			//printing search results
		cout << "\nWould you like to \n 1.   Choose a web page to open\n 2.   New search\n 3.   Exit\n \n \nType in your choice:   ";
		cin >> choice;			//taking input from the user with his choice and search string
		if (choice == 1) {
			cout << "Which webpage you want to search?\n";
			int pageChoice;
			cin >> pageChoice;
			auto it = final.rbegin();
			for (int i = 0;i < pageChoice - 1;i++) {
				it++;				//displaying the wanted page by iterating over all others and then updating its clicks
			}
			cout << "You're now viewing " <<it->second.name << ".\nWould you like to \n 1.   Go back to search results\n 2.   New search\n 3.   Exit\n \n \nType in your choice:   ";
			for (int j = 0;j < page::total;j++) {
				if (it->second.name == adjlist[j][0].name)
					adjlist[j][0].clicks++;
			}
			cin >> choice;
			if (choice == 1)
				choice = 0;			//returning to the begining of the loop with the same search string
		}
		else if (choice == 2) {
			continue;				//returning to the begining of the loop with new search string
		}
	}
	ofstream output;
	output.open("Imp.txt");
	if (output.is_open()) {
		for (int i = 0;i < page::total;i++) {
			if(adjlist[i][0].imp>1)
			output << adjlist[i][0].name << ',' << adjlist[i][0].imp << ',' << adjlist[i][0].clicks << endl;
			else
				output << adjlist[i][0].name << ',' << 0 << ',' << 0 << endl;

		} //updating clicks 
	}
	else	cout << "couldn't save the information";
	cout << "Thank you for using this project";
}
multimap<double, page> calcScore(vector<page> a[], int v, vector<page> x) {
	multimap <double, page> searchResult;
	for (int i = 0; i < x.size(); i++) {
		for (int j = 0;j < v;j++) {
			if (x[i].name == a[j][0].name) {
				a[j][0].pageScore = (0.4 * a[j][0].pageRank) + 0.6 * ((1 - (0.1 * a[j][0].imp) / (1 + 0.1 * a[j][0].imp)) * a[j][0].pageRank + (0.1 * a[j][0].imp) * a[j][0].CTR / (1 + 0.1 * a[j][0].imp));
				searchResult.insert(searchResult.begin(), pair<double, page>( a[j][0].pageScore, a[j][0] ));
				a[j][0].imp++;
			}
		}
	}
	return searchResult;		//looping over all the pages that match the search string, match them with themselves in the adjency list to get their CTR, Rank, and hence their score
	//each page's IMP is updated since it will be displayed to the user
}
void calcRank(vector<page> a[], int v) {
	vector < double > Ranks;
	for (int i = 0; i < v; i++) 
		Ranks.push_back(1.0 / (page::total));	//initializing all page ranks to 1/n
	for (int i = 0; i < v; i++) {
		double sum = 0;
		for (int j = 0; j < v; j++) 
			if (i != j) 
				for (int k = 1;k < a[j].size();k++)
					if (a[i][0].name == a[j][k].name) 
						sum += Ranks[j] / a[j][0].children;		//each page is updated by searching for its parents and calculating their ranks/children
		if (sum == 0)								//the page rank function implementation is done according to https://www.geeksforgeeks.org/page-rank-algorithm-implementation/
			a[i][0].pageRank = Ranks[i];
		else
			a[i][0].pageRank = sum;
	}
}
void calcCTR(vector<page> a[], int v) {
	for (int i = 0; i < v; i++)
		a[i][0].CTR = (a[i][0].clicks * 100.0 / a[i][0].imp); //calcilating CTR for all pages
}
vector<page> doSearch(vector<page> a[], int v, string s) {		//function that returns the pages that matches the search
	vector<page> results;
	if (s.find("AND") > 100) {					//if it does not find AND word it will divide the string into OR/""/ ' '
		vector<string> search = searchStr(s);
		for (int i = 0;i < v;i++) 
			for (int j = 0;j < a[i][0].keywords.size();j++) 
				for (int k = search.size() - 1;k >= 0;k--) 
					if (search[k] == a[i][0].keywords[j]) 
						results.push_back(a[i][0]);
	}
	else {
		int check = 0;
		vector<string> search = searchStrAnd(s); //if it finds AND it will search for it in keywords
		for (int i = 0;i < v;i++)
			for (int j = 0;j < a[i][0].keywords.size();j++)
				for (int k = search.size() - 1;k >= 0;k--)
					if (search[k] == a[i][0].keywords[j]) {
						check++;
						if (check == 2)
							results.push_back(a[i][0]);
					}
	}
	return results;
}
vector<string> searchStrAnd(string s) {
	vector<string> searched;
	string temp;
	temp = s.substr(0, s.find("AND") - 1);
	searched.push_back(temp);
	temp = s.substr(s.find("AND") + 4, s.end() - s.begin());
	searched.push_back(temp);
	return searched;
}
vector<string> searchStr(string s) {
	vector<string> searched;
	string temp;
		if (s.find("OR") < 100) {
			temp = s.substr(0, s.find("OR") - 1);
			searched.push_back(temp);
			s.erase(0, s.find("OR") + 3);
			temp = s;
			searched.push_back(temp);
		}
		else if (s.find(34) < 100) {
			temp = s.substr(1, s.find_last_of('"') - 1);
			searched.push_back(temp);
		}
		else if(s.find(' ')<100) {
			temp = s.substr(0, s.find(" "));
			searched.push_back(temp);
			s.erase(0, s.find(" ")+1);
			temp = s;
			searched.push_back(temp);
		}	
		else {
			searched.push_back(s);
		}
	return searched;
}
void readKey(vector<page> a[]) {
	ifstream input;
	input.open("key.txt");
	if (input.is_open()) {
		string srcO, srcN, key,line;
		int count = -1;
		while (!input.eof()) {
			getline(input, srcN, ',');
			if (srcN != "\n") {
				if (srcN != srcO) {
					count++;
					srcO = srcN;
				}
				getline(input, line);
				stringstream s(line);
				while (getline(s, key, ',')) {
					addKey(a, count, key);
				}
			}
		}
	}
	else	cout << "error";		//reading keywords from the file
}
void addKey(vector<page> adj[], int u, string v) {
	adj[u][0].keywords.push_back(v);	//assigning keywords
}
void printKey(vector<page> adj[], int V) {
	for (int d = 0; d < V; ++d) {
		cout << "\n"
			<< d + 1 << ":";
		for (int x = 0;x < adj[d][0].keywords.size();x++)
			cout << "-> " << adj[d][0].keywords[x];
		cout <<"\n";
	}
}
void readImp(vector<page> a[]) {
	ifstream input;
	input.open("Imp.txt");
	if (input.is_open()) {
		string src, imp, clicks;
		int impn, clicksn;
		int count = 0;
		while (!input.eof()) {
			getline(input, src, ',');
			if (src != "\n") {
				getline(input, imp, ',');
				input >> clicks;
				impn = stoi(imp);
				clicksn = stoi(clicks);
				a[count][0].imp = impn;
				a[count][0].clicks = clicksn;
				count++;
			}
		}
	}				//reading Imp from the file and assigning it to each each page
	else	cout << "error";
}
void printImp(vector<page> adj[], int V) {
	for (int d = 0; d < V; ++d) {
		cout << "\n"
			<< d + 1 << ":";
		cout << adj[d][0].imp;
		cout << "\n";
	}
}
void readGraph(vector<page> a[]) {
	ifstream input;
	input.open("hyper.txt");
	if (input.is_open()) {
		string srcO, srcN, dest;
		int count = -1;
		while (!input.eof()) {
			getline(input, srcN, ',');
			if (srcN != "\n") {
				if (srcN != srcO) {
					count++;
					srcO = srcN;
					page srcn;
					srcn.name = srcN;
					addEdge(a, count, srcn);
				}
				getline(input, dest);
				page destn;
				destn.name = dest;
				addEdge(a, count, destn);
			}
		}
		if (count < created.size() - 1) {
			for (int i =count + 1;i < created.size();i++) {
				addEdge(a, i, created[i]);
			}
		}
	}					//reading links from the file
		else cout << "error";
	
}
void addEdge(vector<page> adj[], int u, page v)
{
	int checker = 0;
	for (int i = 0;i < created.size();i++) {
		if (v.name != created[i].name)
			checker++;
	}
	if (checker == created.size()) {
		created.push_back(v);
		page::total++;
	}
	adj[u].push_back(v);
	adj[u][0].children++;
}					//adding edges to the adjcency list and adding nodes that does not have children
void printGraph(vector<page> adj[], int V) {
	for (int d = 0; d < V; ++d) {
		cout << "\n"
			<< d+1 << ":";
		for (int x=0 ;x< adj[d].size();x++)
			cout << "-> " << adj[d][x].name;
		cout <<"\n";
	}
}
