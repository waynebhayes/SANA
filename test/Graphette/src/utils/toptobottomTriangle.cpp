#include "toptobottomTriangle.hpp"

using namespace std;

void convertToptoBottomTriangle(uint k){
	ifstream fcanon_map("data/canon_map"+to_string(k)+".txt");
	ifstream fcanon_list("data/canon_list"+to_string(k)+".txt");
	uint numGraphette = (pow(2, (k*k-k)/2)+0.5);
	vector<uint> oldtonew, newcanonmap(numGraphette);
	map<uint, vector<uint>> isomorphlist;
	vector<string> permlist(numGraphette);
	uint oldcanon, i = 0;
	string oldperm;
	
	for(; i < numGraphette; i++)
		oldtonew.push_back(convertNum(k, i));
	fcanon_list >> oldcanon;
	while(fcanon_list >> oldcanon){
		isomorphlist[oldcanon] = vector<uint>();
	}
	i = 0;
	while(fcanon_map >> oldcanon >> oldperm){
		isomorphlist[oldcanon].push_back(i);
		permlist[oldtonew[i]] = oldperm;
		i++;
	}
	vector<uint> newcanonlist;
	for(auto& isomorph: isomorphlist){
		for(auto& graph: isomorph.second){
			graph = oldtonew[graph];
		}
		sort(isomorph.second.begin(), isomorph.second.end());
		newcanonlist.push_back(isomorph.second[0]);
		string canonperm = permlist[isomorph.second[0]];
		for(auto graph: isomorph.second){
			permlist[graph] = createPermutation(permlist[graph], canonperm);
			newcanonmap[graph] = isomorph.second[0];
		}
	}
	sort(newcanonlist.begin(), newcanonlist.end());

	ofstream fnewcanon_list("data/newcanon_list"+to_string(k)+".txt");
	fnewcanon_list << isomorphlist.size() << endl;
	for(auto cgraph : newcanonlist)
		fnewcanon_list << cgraph << " ";

	ofstream fnewcanon_map("data/newcanon_map"+to_string(k)+".txt");
	for(i = 0; i < numGraphette; i++)
		fnewcanon_map << newcanonmap[i] << " " << permlist[i] << endl;
	fcanon_list.close();
	fcanon_map.close();
	fnewcanon_list.close();
	fnewcanon_map.close();
}

uint convertNum(uint k, uint n){
	HalfMatrix m(k, n);
	uint sum = 0, l = (k*k-k)/2-1;
	for(uint i = 0; i < k; i++){
		for(uint j = 0; j < i; j++){
			sum += m(i,j)*(pow(2, l)+0.5);
			l--;
		}
	}
	return sum;
}

string createPermutation(string perm1, string perm2){
	string result, rperm2(perm2);
	for(uint i = 0; i < perm2.length(); i++){
		rperm2[perm2[i]-'0'] = ('0'+i);
	}
	for(uint i = 0; i < rperm2.length(); i++){
		result += rperm2[perm1[i]-'0'];
	}
	return result;
}