// example1.cc - loading and dumping an automaton

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

const char* autStr =
	"Ops           a:0 b:2\n"
	"Automaton     aut\n"
	"States        q0 q1\n"
	"Final States  q1\n"
	"Transitions\n"
	"a          -> q0\n"
	"b(q0, q0)  -> q1\n";

typedef VATA::ExplicitTreeAut<unsigned> Automaton;
//typedef VATA::BDDBottomUpTreeAut Automaton;  // uncomment for BDD BU automaton
//typedef VATA::BDDTopDownTreeAut Automaton;   // uncomment for BDD TD automaton

int main()
{
	// create the parser for the Timbuk format
	VATA::Parsing::AbstrParser* parser = new VATA::Parsing::TimbukParser();

	// create the dictionary translating symbol names to internal symbols ...
	typename Automaton::StringToSymbolDict symbolDict;
	// ... and link it to the automaton
	Automaton::SetSymbolDictPtr(&symbolDict);

	// create the ``next symbol'' variable for the automaton
	typename Automaton::SymbolType nextSymbol(0);
	//typename Automaton::SymbolType nextSymbol(16, 0);  // uncomment for BDD aut
	Automaton::SetNextSymbolPtr(&nextSymbol);

	// create the dictionary for translating state names to internal state numbers
	VATA::AutBase::StringToStateDict stateDict;

	// create and load the automaton
	Automaton aut;
	aut.LoadFromString(*parser, autStr, stateDict);

	// create the serializer for the Timbuk format
	VATA::Serialization::AbstrSerializer* serializer =
		new VATA::Serialization::TimbukSerializer();

	// dump the automaton
	std::cout << aut.DumpToString(*serializer, stateDict);
}
