//Max Blankestijn s1819658 & Loes Dekker s1850024

#include "Automaton.h"

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

void Automaton::eliminateLambda(Automaton& fa) {
		if(fa.alphabet.size() == 0) { //no free vars
			//in dit geval is er 1 inital state die ook accepting is als de formule
			//True is en maximaal 1 transitie (naar zichzelf)
				fa.transitions.clear(); //verwijder deze, geen andere transities
		}
}

// void Automaton::eliminateLambda(Automaton& fa) {
// 	State temp;
// 	Automaton aut = fa;
// 	clearAuto();
// 	std::set<State> done;//staten die behandeld zijn en al in temp verwerkt zijn
// 	bool init, fin = false;
// 		for(auto i: aut.transitions) {
// 			if(done.find(i.first) == done.end()) {//al gemerged, niet afhandelen
// 				temp = i.first; done.insert(i.first);
// 				if(aut.initialStates.find(temp) != aut.initialStates.end()) init = true;
// 				if(aut.finalStates.find(temp) != aut.finalStates.end()) fin = true;
// 				for(auto j : i.second) {//ga door maps
// 					for(auto k : j.second) {//ga door set ints
// 						if(done.find(k) == done.end()) {//is k al verwerkt in een state?
// 							temp = cantor(temp, k);//alles mergen waar je heen kan met lamda
// 							done.insert(k);
// 							if(aut.initialStates.find(temp) != aut.initialStates.end()) init = true;
// 							if(aut.finalStates.find(temp) != aut.finalStates.end()) fin = true;
// 						}
// 					}
// 				}
// 				addState(temp);
// 				if(init) markInitial(temp);//deel van temp was init, dus geheel ook
// 				if(fin) markFinal(temp);
// 				init = fin = false;
// 			}
// 		}
// }

// void Automaton::makeDeterministic(Automaton& fa) {
// 		fa.eliminateLambda(fa);
// 		std::set<State> newStates;
// 		std::set<State> goesTo; //samen te voegen tot nieuwe staat
// 		std::map<BitVector, std::set<State> > tempMap;
// 		std::map<State, std::map<BitVector, std::set<State> > > newTrans;
// 		for(auto from : fa.states) {
// 				for(auto trans : fa.transitions[from]) { //itereer door transities
// 						for(auto to : trans.second)//itereer door set to-states
// 							goesTo.insert(to); //merge deze states tot 1
// 							newStates.clear(); newStates.insert(/*merged states*/);
// 							tempMap.clear();
// 							tempMap.insert(std::pair<Bitvector, std::set<State>>(trans.first, newStates));//met bitvec naar gemergede state
// 							newTrans.insert(std::pair<State, std::map<State, std::set<State>>>
// 															(from, tempMap));
// 				}
// 		}
// 		//for every state
// 			//make trans state -> bv -> result of every bv-transition (dont if empty)
// }

void Automaton::makeDeterministic(Automaton& fa) {
		fa.eliminateLambda(fa);
		std::set<State> newStates;
		std::set<State> visited;
		std::map<State, std::map<BitVector, std::set<State> > > newTrans;
		for(auto i : fa.initialStates) {
			recDet(fa, visited, newTrans, newStates);
		}
		//for every initialize
			//call recursion function(Automaton, list/set::Visited&, int::newState):
				//for every pair <bitvector and set of States>
					//if current not in visited-list
						//add current to list of visited states
						//for every transition
							//merge set of states into set of one states
							//add transition from current with bitv to set
							//call recursion on destintion of created transition
}

void Automaton::recDet(Automaton fa, std::set &visited, std::set<State> &states, int newState) {
		int merged;


		if(visited.find(newState) != visited.end()) {
			visited.insert(newState); fa.states.insert(newState);
			for(auto state : originalStates) //bepaal alle originele states verwerkt in newState
				for(auto trans : fa.transitions[state]) { //itereer door transities

							tempMap.clear();
							tempMap.insert(std::pair<Bitvector, std::set<State>>(trans.first, newStates));//met bitvec naar gemergede state
							newTrans.insert(std::pair<State, std::map<State, std::set<State>>>
															(from, tempMap));
			//merge fa.transitions ?? nog zien te fixen


		}
	}

void Automaton::quant(unsigned var){
	BitVector bv;
	std::map<BitVector, std::set<State>> tempmap;
	std::map<State, std::map<BitVector, std::set<State>>> newTransitions;
	for (auto i : transitions){
		for(auto j : i.second){
			bv = j.first;
			bv.erase(var);
			tempmap.insert(std::pair<BitVector, std::set<State>>
				(bv, transitions[i.first][j.first]));
		}
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
	for(auto i : alphabet) {
		temp.insert(std::pair<unsigned, bool>(i, 0));
	}
	next(temp);
	return this;
}
