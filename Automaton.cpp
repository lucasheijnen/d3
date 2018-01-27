//Max Blankestijn s1819658 & Loes Dekker s1850024

#include "Automaton.h"
#include <queue>

// TODO (voor studenten - deel 1): VOEG HIER DE IMPLEMENTATIES VAN DE OPERATIES IN Automaton.h TOE

void Automaton::addState(const State state){
	if(states.find(state) == states.end()) states.insert(state);
}

void Automaton::addTransition
	(const State from, const BitVector label, const State to){
	transitions[from][label].insert(to);
}

void Automaton::markInitial(const State state){
	initialStates.insert(state);
	currentStates.insert(state);
}

void Automaton::markFinal(const State state){
	if(states.find(state) != states.end() &&
			finalStates.find(state) == finalStates.end()) finalStates.insert(state);
}

void Automaton::parseInput(const std::list<BitVector> input){
	std::set<State> temp;
	currentStates = initialStates;
	for(auto i : input)	next(i);
}

bool Automaton::inFinalState() const{
	for(auto const i : currentStates)
		if(finalStates.find(i) != finalStates.end()) return true;
	return false;
}

int cantor(unsigned int num1, unsigned int num2){
	return (.5*(num1+num2)*(num1+num2+1)+num2);
}

void invCan(unsigned int num, int &num1, int &num2){
	double temp1, temp2;
	temp1 = floor((pow(8*num+1,.5)-1)/2);
	temp2 = (pow(temp1, 2) +  temp1) /2;
	num2 = num - temp2;
	num1 = temp1 - num2;
}

void Automaton::clearAuto(){
	states.clear();
	initialStates.clear();
	finalStates.clear();
	transitions.clear();
	currentStates.clear();
	alphabet.clear();
}

void Automaton::intersect(Automaton& fa1, Automaton& fa2){
	std::map<BitVector, std::set<State> > temp1, temp2;
	int num1, num2;
	clearAuto();
	alphabet = fa1.alphabet;
	for(auto i : fa1.initialStates)
		for(auto j : fa2.initialStates)
			{addState(cantor(i, j)); markInitial(cantor(i, j));}
	std::set<State> remain(states);
	while(remain.size() != 0){
		State newState = *remain.begin();
		remain.erase(remain.begin());
		addState(newState);
		invCan(newState, num1, num2);
		if(fa1.finalStates.find(num1) != fa1.finalStates.end() &&
		   fa2.finalStates.find(num2) != fa2.finalStates.end())
			markFinal(newState);
		temp1 = fa1.transitions[num1];
		temp2 = fa2.transitions[num2];
		for(auto i : temp1)
			for(auto j : temp2[i.first])
				for(auto k : i.second){
					if(states.find(cantor(k, j)) == states.end())
						remain.insert(cantor(k, j));
					addTransition(newState, i.first, cantor(k, j));
				}
	}
}

void Automaton::addToAlphabet(unsigned varnr){
	BitVector newbv;
	std::map<State, std::map<BitVector, std::set<State> > > newTransitions;
	if(alphabet.find(varnr) == alphabet.end()){
		for(auto i : transitions)
			for(auto j : transitions[i.first]){
				newbv = j.first;
				newbv[varnr] = 0;
				newTransitions[i.first].insert(std::pair<BitVector,
				std::set<State>>(newbv, j.second));
				newbv[varnr] = 1;
				newTransitions[i.first].insert(std::pair<BitVector,
				std::set<State>>(newbv, j.second));
			}
		alphabet.insert(varnr);
		transitions = newTransitions;
	}
}

void Automaton::next(const BitVector input){
	std::set<State> temp = currentStates;
	currentStates.clear();
	for(auto i : temp)
		if(transitions[i].find(input) != transitions[i].end())
			currentStates.insert(
			transitions[i][input].begin(), transitions[i][input].end());
}

void Automaton::printStates(std::ostream &str, const std::set<State> s) {
    str << "{";
    for(auto& st : s) {
        str << st << ", ";
    }
    str << "}";

}

void Automaton::printTransitionLabel(std::ostream &str, const BitVector t) {
    str << "{";
    for(auto& bv : t) {
        str << bv.first << "->" << bv.second << ", ";
    }
    str << "}";
}

void Automaton::print(std::ostream &str) const {
    str << "Free variables: {";
    for(const int var: alphabet) {
        str << "x" << var << ", ";
    }
    str << "States: ";
    printStates(str, states);
    str << " Initial States: ";
    printStates(str, initialStates);
    str << " Final States: ";
    printStates(str, finalStates);
    str << " Current States: ";
    printStates(str, currentStates);
    str <<"\nTransitions: \n";

    for (auto& trans : transitions) {
        for(auto& m : trans.second) {
            str << "(";

            // print label of source state
            str << trans.first;
            str << ", ";

            // print label of transition
            printTransitionLabel(str, m.first);
            str << ", ";

            // print target states
            printStates(str, m.second);
            str << ")\n";
        }
    }
}

bool Automaton::recLam(Automaton& fa, std::set<State>& visited, State state) {
	if(fa.finalStates.find(state) != fa.finalStates.end()) return true;
	visited.insert(state);
	for(auto trans : fa.transitions[state]) {
			for(auto dest : trans.second)
				if(visited.find(dest) == visited.end() && recLam(fa, visited, dest))
					return true;
	}
	return false;
}

void Automaton::eliminateLambda(Automaton& fa) {
		std::set<State> visited;
		std::set<State> newFinal;
		std::set<State> newStates;
		if(fa.alphabet.size() == 0) { //no free vars
			for(auto i : initialStates) {
				newStates.insert(i);
				if (recLam(fa, visited, i))
					newFinal.insert(i);
		}
		fa.states = newStates;
		fa.finalStates = newFinal;
		fa.transitions.clear();
	}
}

void Automaton::makeDeterministic(Automaton& fa) {
		fa.eliminateLambda(fa);
		std::set<State> visited;
		std::map<State, std::map<BitVector, std::set<State> > > newTrans;
		std::queue<State> Q;
		alphabet = fa.alphabet;
		for(auto i : fa.initialStates) {
			recDet(fa, visited, newTrans, i, Q);
		}
		states = visited;
		transitions = newTrans;
		initialStates = currentStates = fa.initialStates;
		if(visited.find(-1) != visited.end()){ //thrash state
			int temp;
			BitVector bv;
			for(int i = 0; i != pow(2, alphabet.size()); ++i){
				temp = i;
				for(auto j : alphabet){
					bv[j] = temp&1;
					temp >>= 1;
				}
				transitions[-1][bv].insert(-1);
			}
		}
}

State Automaton::merge(std::set<State> mstates, State largest){
	if(mstates.size() == 1) return *mstates.begin();	
	State temp;
	temp = *mstates.begin();
	for(std::set<State>::iterator i = std::next(mstates.begin()); i != mstates.end(); ++i)		
		temp = cantor(temp, *i);
	return temp + largest;
}

std::set<State> Automaton::unmerge(State state, State largest, std::set<State> oregeno){
	std::set<State> tempSet;
	if(oregeno.find(state) != oregeno.end()) tempSet.insert(state);
	else {
		int num1, num2;
		num1 = state - largest;
		do{
			invCan(num1, num1, num2);
			tempSet.insert(num2);
		}
		while(oregeno.find(num1) == oregeno.end());
		tempSet.insert(num1);
	}
	return tempSet;
}

void Automaton::recDet(Automaton fa, std::set<State> &visited,
							std::map<State, std::map<BitVector, std::set<State> > >& newTrans,
							State newState, std::queue<State> &Q) {
		State merged;
		BitVector bv; int temp;
		std::set<State> newStates; //set voor de nieuwe transitie
		std::set<State> originalStates;
		std::map<BitVector, std::set<State>> tempMap;
		if(visited.find(newState) == visited.end()) {
			originalStates = unmerge(newState, *prev(fa.states.end()), fa.states);
			visited.insert(newState); //states.insert(newState);
			for(auto state : originalStates){ //bepaal alle originele states verwerkt in newState
				if(fa.finalStates.find(state) != fa.finalStates.end()) 
					finalStates.insert(newState);
				for(auto trans : fa.transitions[state]) //itereer door transities
						for(auto i : trans.second) //itereer door set states 
							tempMap[trans.first].insert(i);//voeg alle states toe aan set in tempMap
			}
			//tempMap bevat BV->alle states die bereikt kunnen worden uit de originalStates
			for(int i = 0; i != pow(2, fa.alphabet.size()); ++i){
				temp = i;
				for(auto j : fa.alphabet){
					bv[j] = temp&1;
					temp >>= 1;
				}
				if(tempMap.find(bv) == tempMap.end()) {
					if(visited.find(-1) == visited.end()) visited.insert(-1);
					tempMap[bv].insert(-1);
				}
				else {
				merged = merge(tempMap[bv], *prev(fa.states.end()));
				Q.push(merged);
				tempMap[bv].clear(); tempMap[bv].insert(merged); //transitie met alleen merged = deterministic
				}
			}
			newTrans.insert(std::pair<State, std::map<BitVector, std::set<State>>>(newState, tempMap));
			while(!Q.empty()){//states die bereikt kunnen worden vanaf deze ook verwerken
				newState = Q.front(); Q.pop();
				recDet(fa, visited, newTrans, newState, Q);
			}
		}
}

void Automaton::quant(unsigned var){
    BitVector bv;
    std::set<State> tempset;
    std::map<BitVector, std::set<State>> tempmap;
    std::map<State, std::map<BitVector, std::set<State>>> newTransitions;
    for (auto i : transitions){
        for(auto j : i.second){
            bv = j.first;
            bv.erase(var);
            if(bv.empty()) tempset.insert(transitions[i.first][j.first].begin(),
                                                                        transitions[i.first][j.first].end());
            else tempmap.insert(std::pair<BitVector, std::set<State>>
                (bv, transitions[i.first][j.first]));
        }
        if(!tempset.empty()) tempmap.insert(std::pair<BitVector, std::set<State>>
                                                    (bv, tempset));
        newTransitions.insert(std::pair<State,
            std::map<BitVector, std::set<State>>>(i.first, tempmap));
        tempmap.clear(); ;
    }
    alphabet.erase(var);
    transitions = newTransitions;
}


bool Automaton::stateInstates(State state){
	if(states.find(state) == states.end()) return false;
	return true;
}

void Automaton::addVar(unsigned x){
	alphabet.insert(x);
}

void Automaton::insertFreeVars(Automaton fa2){
	for(auto i : fa2.alphabet)
		if(alphabet.find(i) == alphabet.end()) addToAlphabet(i);
}

Automaton* Automaton::nulBit(){
	BitVector temp;
	std::set<State> visited;
	bool loop = false;
	for(auto i : alphabet)
		temp.insert(std::pair<unsigned, bool>(i, 0));
	while(!inFinalState() & !loop){
		visited.insert(currentStates.begin(), currentStates.end());
		next(temp);
		loop = true;
		for(auto i : currentStates)
			if(visited.find(i) == visited.end()) loop = false;
	}
	return this;
}

void Automaton::comp(){
	std::set<State> newFinal;
	for(auto i : states)
		if(finalStates.find(i) == finalStates.end()) newFinal.insert(i);
	finalStates = newFinal;
}
