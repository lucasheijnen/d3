//Max Blankestijn s1819658 & Loes Dekker s1850024

#include "Automaton.h"

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
	BitVector newBv;
	std::map<State, std::map<BitVector, std::set<State> > > newTransitions;
	if(alphabet.find(varnr) == alphabet.end()){
		for(auto i : transitions)
			for(auto j : transitions[i.first]){
				newBv = j.first;
				newBv[varnr] = 0;
				newTransitions[i.first].insert(std::pair<BitVector,
				std::set<State>>(newBv, j.second));
				newBv[varnr] = 1;
				newTransitions[i.first].insert(std::pair<BitVector,
				std::set<State>>(newBv, j.second));
			}
		alphabet.insert(varnr);
		transitions = newTransitions;
	}
}

void Automaton::next(const BitVector in){
	std::set<State> temp = currentStates;
	currentStates.clear();
	for(auto i : temp)
		if(transitions[i].find(in) != transitions[i].end())
			currentStates.insert(transitions[i][in].begin(),transitions[i][in].end());
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
	if(fa.alphabet.size() == 0) { //no free vars
		for(auto i : initialStates) {
			states.insert(i);
			if (recLam(fa, visited, i))
				finalStates.insert(i);
		}
		transitions.clear();
	}
}

void Automaton::makeDeterministic(Automaton& fa) {
	std::set<State> visited;
	std::map<State, std::map<BitVector, std::set<State> > > newTrans;
	std::queue<State> Q;
	alphabet = fa.alphabet;
	int temp;
	BitVector bv;
	fa.eliminateLambda(fa);
	for(auto i : fa.initialStates) recDet(fa, visited, newTrans, i, Q);
	states = visited;
	transitions = newTrans;
	initialStates = currentStates = fa.initialStates;
	if(visited.find(-1) != visited.end()) //thrash state
		for(int i = 0; i != pow(2, alphabet.size()); ++i){
			temp = i;
			for(auto j : alphabet){
				bv[j] = temp&1;
				temp >>= 1;
			}
			transitions[-1][bv].insert(-1);
		}
}

State Automaton::merge(std::set<State> mStates, State largest) const{
	if(mStates.size() == 1) return *mStates.begin(); //not enough states to merge
	State temp;
	temp = *mStates.begin();
	for(std::set<State>::iterator i = std::next(mStates.begin()); i != mStates.end(); ++i)
		temp = cantor(temp, *i); //merge all states in set with cantor pairing
	return temp + largest; //to avoid overlap with already existing state
}

std::set<State> Automaton::unmerge(State state, std::set<State> original) const {
	std::set<State> tempSet;
	int num1, num2;
	if(original.find(state) != original.end()) tempSet.insert(state);//state wasn't merged
	else {
		num1 = state - *prev(original.end());
		do {
			invCan(num1, num1, num2);//unmerge states until all original states were found
			tempSet.insert(num2);
		} while(original.find(num1) == original.end());
		tempSet.insert(num1);
	}
	return tempSet;
}

void Automaton::recDet(Automaton fa, std::set<State> &visited,
							std::map<State, std::map<BitVector, std::set<State> > >& newTrans,
							State newState, std::queue<State> &Q) {
	State merged;
	BitVector bv;
	int temp;
	std::set<State> originalStates, newStates; //set voor de nieuwe transitie
	std::map<BitVector, std::set<State>> tempMap;
	if(visited.find(newState) == visited.end()) {
		originalStates = unmerge(newState, fa.states);
		visited.insert(newState); //states.insert(newState);
		for(auto state : originalStates){ //bepaal alle originele states verwerkt in newState
			if(fa.finalStates.find(state) != fa.finalStates.end())
				finalStates.insert(newState);
			for(auto trans : fa.transitions[state]) //itereer door transities
				for(auto i : trans.second) //itereer door set states
					tempMap[trans.first].insert(i);//voeg alle states toe aan set in tempMap
		}//tempMap bevat BV->alle states die bereikt kunnen worden uit de originalStates
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
				tempMap[bv].clear(); tempMap[bv].insert(merged);
				} //transitie met alleen merged = deterministic
		}
		newTrans.insert(std::pair<State, std::map<BitVector, std::set<State>>>(newState, tempMap));
		while(!Q.empty()){//states die bereikt kunnen worden vanaf deze ook verwerken
			newState = Q.front(); Q.pop();
			recDet(fa, visited, newTrans, newState, Q);
		}
	}
}

void Automaton::project(const unsigned variable){
  BitVector bv;
  std::set<State> tempSet;
  std::map<BitVector, std::set<State>> tempMap;
  std::map<State, std::map<BitVector, std::set<State>>> newTransitions;
  for (auto i : transitions){
    for(auto j : i.second){
      bv = j.first;
      bv.erase(variable); //projected variable is erased from temp bitVector
      if(bv.empty())
      	tempSet.insert(transitions[i.first][j.first].begin(),
       		transitions[i.first][j.first].end()); //sets that can be reached with
      else tempMap.insert(std::pair<BitVector, std::set<State>> //lambda transition
       (bv, transitions[i.first][j.first])); //sets that can be reached with
    }																				 //'new' bitVector
    if(!tempSet.empty()) //no lambda transitions
     	tempMap.insert(std::pair<BitVector, std::set<State>>(bv, tempSet));
      newTransitions.insert(std::pair<State,
        std::map<BitVector, std::set<State>>>(i.first, tempMap));
      tempMap.clear();
  }
  alphabet.erase(variable);
  transitions = newTransitions;
  eliminateLambda(*this);
}

bool Automaton::contains(State state) const{
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

void Automaton::complement(Automaton& fa){
	std::set<State> newFinal;
	for(auto i : fa.states) //all non final states are inserted into newFinal
		if(fa.finalStates.find(i) == fa.finalStates.end()) newFinal.insert(i);
	finalStates = newFinal;
	states = fa.states; //rest of the given automaton fa is copied into this
	transitions = fa.transitions; //automaton
	initialStates = fa.initialStates;
	currentStates = fa.currentStates;
}

void Automaton::restate(){
	int count = 0; //new name of first state
	std::set<State> newStates, newFin, newInit, newCur;
	std::map<State, State> fromTo;
	std::map<BitVector, std::set<State>> tempMap;
	std::map<State, std::map<BitVector, std::set<State>>> newTrans;
	for(auto i : states) {
		newStates.insert(count);
		if(finalStates.find(i) != finalStates.end()) newFin.insert(count);
		if(initialStates.find(i) != initialStates.end()) newInit.insert(count);
		if(currentStates.find(i) != currentStates.end()) newCur.insert(count);
		fromTo.insert(std::pair<State, State>(i, count)); //new state to replace
		count++;																					//old state are saved
	}
	states = newStates;
	finalStates = newFin;
	initialStates = newInit;//all set of states are replaced with sets containing
	currentStates = newCur; //with set containing new names
	for(auto j : transitions){
		for(auto k : j.second)
			for(auto l : k.second)
				tempMap[k.first].insert(fromTo[l]); //bitvector to new state
		newTrans.insert(std::pair<State, std::map<BitVector, std::set<State>>>
			(fromTo[j.first], tempMap)); //new state to tempMap is inserted
		tempMap.clear();
	}
	transitions = newTrans; //transitions containing old states is replaced with
}													//set containing new states
