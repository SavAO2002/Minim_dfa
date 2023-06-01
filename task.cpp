#include "api.hpp"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <stack>
#include <iostream>

using namespace std;

void rewriting(map <int, map <string, map <char, int>>> &table, map <string, int> new_states, DFA d) {
	for (auto state = table.begin(); state != table.end(); state ++) {
		for (auto j = state->second.begin(); j != state->second.end(); j++) {
			for (auto sym = j->second.begin(); sym != j->second.end(); sym++) {
				table[state->first][j->first][sym->first] = new_states[d.get_trans(j->first, sym->first)];
			}
		}
	}
}

DFA dfa_minim(DFA &d) {
	string trans = (d.get_alphabet()).to_string();
	set <string> names = d.get_states();
	set <string> unreach_names;

	string initial_state = d.get_initial_state();
	cout << '*' << endl;
	stack <string> states;
	states.push(initial_state);
	unreach_names.insert(initial_state);
	while (!states.empty()) {      //удаление недостижимых состояний
		string state = states.top();
		states.pop();
		for (auto i = trans.begin(); i != trans.end(); i++) {
			if (d.has_trans(state, *i)) {
				string name = d.get_trans(state, *i);
				if (unreach_names.count(name) == 0) {
					states.push(name);
					unreach_names.insert(name);
				}
			}
		}
	}
	for (auto i : names) {
		if (unreach_names.count(i) == 0) {
			d.delete_state(i);
		}
	}

	d.create_state("deadlock");
	names = d.get_states();
	map <int, map <string, map <char, int>>> table;
 
	map <string, int> new_states;
	for (auto i : names) {   //деление на финальные и не очень
		if (d.is_final(i)) {
			new_states[i] = 1;
		}
		else {
			new_states[i] = 0;
		}
	}
	new_states["deadlock"] = 0;
	//cout << new_states << endl;

	for (auto i = names.begin(); i != names.end(); i++) {  //заполнение таблицы и добавление переходов в мертвое состояние
		if (d.is_final(*i)) {
			for (auto j = trans.begin(); j != trans.end(); j++) {
				if (!d.has_trans(*i, *j)) {
					d.set_trans(*i, *j, "deadlock");
					table[1][*i][*j] = 0;
				}
				else {
					table[1][*i][*j] = new_states[d.get_trans(*i, *j)];
				}
			}
		}
		else {
			for (auto j = trans.begin(); j != trans.end(); j++) {
				if (!d.has_trans(*i, *j)) {
					d.set_trans(*i, *j, "deadlock");
					table[0][*i][*j] = 0;
				}
				else {
					table[0][*i][*j] = new_states[d.get_trans(*i, *j)];
				}
			}
		}
	}
	int last_num = 2;
	//cout << table << endl;
	bool flag = 1;
	while (flag) {
		flag = 0;
		for (auto i = table.begin(); i != table.end(); i++) {
			auto elem1 = i->second.begin();
			auto j = elem1;
			j++;
			map <string, map <char, int>> tmp;
			while (j != i->second.end()) {
				if (j->second != elem1->second) {
					flag = 1;
					tmp[j->first] = j->second;
					new_states[j->first] = last_num;
					j = i->second.erase(j);
				}
				else {
					j++;
				}
			}
			if (!tmp.empty()) {
				table[last_num] = tmp;
			}
			rewriting(table, new_states, d);
			//cout << '*' << endl;
			last_num += 1;
		}
	}
	//cout << table << endl;
	//cout << new_states << endl;

	DFA new_d = DFA(trans);
	string dead_state = to_string(new_states["deadlock"]);
	for (auto i = table.begin(); i != table.end(); i++) {
		string name_state = to_string(i->first);
		if (name_state != dead_state) {
			new_d.create_state(name_state);
			if (d.is_final(table[i->first].begin()->first)) {
				new_d.make_final(name_state);
			}
		}
	}
	for (auto i = table.begin(); i != table.end(); i++) {
		string name_state = to_string(i->first);
		if (name_state != dead_state) {
			auto j = i->second.begin();
			for (auto k = j->second.begin(); k != j->second.end(); k++) {
				cout << to_string(k->second) << ' ' << name_state << ' ' << (k->second != new_states["deadlock"]) << endl;
				if (k->second != new_states["deadlock"]) {
					new_d.set_trans(name_state, k->first, to_string(k->second));
					cout << new_d.has_trans(name_state, k->first) << endl;
				}
			}
		}
	}

	new_d.set_initial(to_string(new_states[initial_state]));

	return new_d;
}
