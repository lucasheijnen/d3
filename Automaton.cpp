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
	finalStates.insert(state);
}

void Automaton::parseInput(const std::list<BitVector> input){
	std::set<State> temp;
	for(std::list<BitVector>::const_iterator l = input.begin(); 
		l != input.end(); ++l){
		for(std::set<State>::iterator i = currentStates.begin(); 
			i != currentStates.end(); ++i)
			temp.insert(transitions[*i][*l].begin(), transitions[*i][*l].end());
		currentStates = temp;
		temp.clear();
	}
}

bool Automaton::inFinalState() const{
	for(std::set<State>::const_iterator i = currentStates.begin(); 
			i != currentStates.end(); ++i)
		if(finalStates.find(*i) != finalStates.end()) return true;	
	return false;
}

int cantor(int num1, int num2){
	return (.5*(num1+num2)*(num1+num2+1)+num2);
}

void invCan(int num, int &num1, int &num2){
	double temp1, temp2; 
	temp1 = floor((pow(8*num+1,.5)-1)/2); 
	temp2 = (pow(temp1, 2) +  temp1) /2; 
	num2 = num - temp2; 
	num1 = temp1 - num2; 
}

void Automaton::intersect(Automaton& fa1, Automaton& fa2){
	std::map<State, std::map<BitVector, std::set<State> > > newTransitions;
	std::set<State> newStates, newInitStates, newFinitStates, remain;
	std::map<BitVector, std::set<State> > temp1, temp2;
	int num1, num2;
	for(std::set<State>::iterator i = fa1.initialStates.begin(); 
		i != fa1.initialStates.end(); ++i)
		for(std::set<State>::iterator j = fa2.initialStates.begin();
			j != fa2.initialStates.end(); ++j)
			newInitStates.insert(cantor(*i, *j));
	remain = newInitStates;
	while(remain.size() != 0){
		State newState = *remain.begin();
		remain.erase(remain.begin());
		newStates.insert(newState);
		//std::cout << newState << std::endl;
		invCan(newState, num1, num2);
		if(fa1.finalStates.find(num1) != fa1.finalStates.end() &&
		   fa2.finalStates.find(num2) != fa2.finalStates.end())
			newFinitStates.insert(newState);
		temp1 = fa1.transitions[num1];
		temp2 = fa2.transitions[num2];
		for(std::map<BitVector, std::set<State> > ::iterator i = temp1.begin();
			i != temp1.end(); ++i){
			for(std::set<State>::iterator j = temp2[i->first].begin(); 
				j != temp2[i->first].end(); ++j){
				for(std::set<State>::iterator k = i->second.begin();
					k != i->second.end(); ++k){
					if(newStates.find(cantor(*k, *j)) == newStates.end()){
						remain.insert(cantor(*k, *j));
					} 
					newTransitions[cantor(num1, num2)][i->first].insert(cantor(*k, *j));
				}
			}
		}
	}
	states = newStates;
	initialStates = newInitStates;
	currentStates = initialStates;
	finalStates = newFinitStates;
	transitions = newTransitions;
}

void Automaton::addToAlphabet(unsigned varnr){ 
	BitVector newbv; 
	std::map<State, std::map<BitVector, std::set<State> > > newTransitions;
	if(alphabet.find(varnr) == alphabet.end())
		for(std::map<State, std::map<BitVector, std::set<State>>>::iterator i = 
			transitions.begin(); i != transitions.end(); ++i)
			for(std::map<BitVector, std::set<State>>::iterator j = 
				transitions[i->first].begin(); j != transitions[i->first].end();
				++j){
				newbv = j->first;
				newbv[varnr] = 0;
				newTransitions[i->first].insert(std::pair<BitVector,
				std::set<State>>(newbv, j->second));
				newbv[varnr] = 1;
				newTransitions[i->first].insert(std::pair<BitVector,
				std::set<State>>(newbv, j->second));
			}
	transitions = newTransitions;
}

void Automaton::next(const BitVector input){
	std::set<State> temp = currentStates;
	currentStates.clear();
	for(std::set<State>::iterator i = temp.begin(); i != temp.end(); ++i){
		currentStates.insert(
		transitions[*i][input].begin(), transitions[*i][input].end());
	}
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

