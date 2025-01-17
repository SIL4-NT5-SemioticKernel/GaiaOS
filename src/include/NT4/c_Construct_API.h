//It annoys the shit out of me to put the count before the array in the argument list, but the text server is structured as such, mostly because I'm being lazy and don't want to write a complex interpreter atm, so this is structured as such.

//The construct encapsulates the node network, the CANs, I/O, granulation filter, uinterface, actuator interface, and I/O tables.
//The result is that after setting up the network the user can use it like a black box.
class c_Construct_API
{
public:
	
	c_Construct Base;

	c_Construct_API()
	{
	}

	//Checks for a construct by name, if found returns the ID, if not found returns -1
	int get_Construct_ID(std::string p_Construct_Name)
	{
		//std::cout << "\n p_Construct_Name: " << p_Construct_Name << " ID: " << Base.get_Construct_ID(p_Construct_Name);
		return Base.get_Construct_ID(p_Construct_Name);
	}

//      ---==================---
//     ---====================---
//    ---======================---
//   ---========================---
//  ---==   NT4 Deep Control   ==---
//   ---========================---
//    ---======================---
//     ---====================---
//      ---==================---
  
	void save_Config(std::string p_Construct)
	{
		Base.save_Config(get_Construct_ID(p_Construct));
	}

	void update_Config(std::string p_Construct)
	{
		Base.update_Config(get_Construct_ID(p_Construct));
	}

	//Hyperparams
	void set_Base_Charge(std::string p_Construct, double p_Base_Charge)
	{
		Base.set_Base_Charge(get_Construct_ID(p_Construct), p_Base_Charge);
	}

	void set_Modifier_Charge(std::string p_Construct, double p_Modifier_Charge)
	{
		Base.set_Modifier_Charge(get_Construct_ID(p_Construct), p_Modifier_Charge);
	}

	void set_Action_Potential_Threshold(std::string p_Construct, double p_Action_Potential_Threshold)
	{
		Base.set_Action_Potential_Threshold(get_Construct_ID(p_Construct), p_Action_Potential_Threshold);
	}

	void set_Charging_Tier(std::string p_Construct, int p_Charging_Tier)
	{
		Base.set_Charging_Tier(get_Construct_ID(p_Construct), p_Charging_Tier);
	}

	double get_Base_Charge(std::string p_Construct)
	{
		return Base.get_Base_Charge(get_Construct_ID(p_Construct));
	}

	double get_Modifier_Charge(std::string p_Construct)
	{
		return Base.get_Modifier_Charge(get_Construct_ID(p_Construct));
	}

	double get_Action_Potential_Threshold(std::string p_Construct)
	{
		return Base.get_Action_Potential_Threshold(get_Construct_ID(p_Construct));
	}

	int get_Output_Depth(std::string p_Construct)
	{
		return Base.get_Output_Depth(get_Construct_ID(p_Construct));
	}

	float get_Output_Charge(std::string p_Construct, int p_Output)
	{
		return Base.get_Output_Charge(get_Construct_ID(p_Construct), p_Output);
	}

	uint64_t get_Output_Treetop_NID(std::string p_Construct, int p_Output)
	{
		return Base.get_Output_Treetop_NID(get_Construct_ID(p_Construct), p_Output);
	}

	float get_Output_RC(std::string p_Construct, int p_Output)
	{
		return Base.get_Output_RC(get_Construct_ID(p_Construct), p_Output);
	}

	std::string get_Output_Pattern(std::string p_Construct, int p_Output)
	{
		return Base.get_Output_Pattern(get_Construct_ID(p_Construct), p_Output);
	}

	std::vector<uint64_t> get_Output_Pattern_uint(std::string p_Construct, int p_Output)
	{
		return Base.get_Output_Pattern_uint(get_Construct_ID(p_Construct), p_Output);
	}
	
	uint64_t get_Output_Primitive_uint(std::string p_Construct, int p_Output, int p_Index = 0)
	{
		return Base.get_Output_Primitive_uint(get_Construct_ID(p_Construct), p_Output, p_Index);
	}

	int get_Output_Pattern_Size(std::string p_Construct, int p_Output)
	{
		return Base.get_Output_Pattern_Size(get_Construct_ID(p_Construct), p_Output);
	}

	int get_Construct_Count()
	{
		return Base.get_Construct_Count();
	}

//    ---==========---
//   ---============---
//  ---==   Node   ==---
//   ---============---
//    ---==========---

	//---==  set_type [NID] [TYPE]  ==---//
    //(0: State), (1: Branch), (2: Treetop), (3: State/Treetop)
	void set_Type(uint64_t p_NID, uint8_t p_Type)
	{
		Base.set_Type(Base.get_Node_Ref_By_NID(p_NID), p_Type);
	}


	
	//---==  add_axon_index [NID] [Axon_NID] [INDEX]  ==---//
	//Adds an axon to the axon list at the given index, if the index doesn't exist then exist it with resize_Axon_Hillocks()
	void add_Axon_Index(uint64_t p_NID, uint64_t p_Axon_NID, int p_Index)
	{
		Base.add_Axon_Index(Base.get_Node_Ref_By_NID(p_NID), Base.get_Node_Ref_By_NID(p_Axon_NID), p_Index);
	}


	
	//---==  set_Dendrites [NID] [DENDRITE_IDS[]] [COUNT]  ==---//
    //Sets the dendrites of the node.
    //This assumes the node has no dendrites yet, if it does you be dangling and jangling
	void set_Dendrites(uint64_t p_NID, int p_Count, uint64_t* p_Dendrite_NIDs)
	{
		c_Node** tmp_Dendrites = NULL;
		tmp_Dendrites = new c_Node*[p_Count];

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Dendrites[cou_Index] = Base.get_Node_Ref_By_NID(p_Dendrite_NIDs[cou_Index]);
		}

		Base.set_Dendrites(Base.get_Node_Ref_By_NID(p_NID), p_Count, tmp_Dendrites);

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Dendrites[cou_Index] = NULL;
		}

		if (tmp_Dendrites != NULL) { delete[] tmp_Dendrites; tmp_Dendrites = NULL; }
	}


	//---==  does_Upper_Tier_Connection_Exist NODE_COUNT NODES[]  ==---//
    //Searches the axons to see if an upper tier connection exists.
    //This is always called from the first leg, that is why we separate _F from normal.
	uint64_t does_Upper_Tier_Connection_Exist(int p_Count, uint64_t* p_Nodes)
	{
		c_Node** tmp_Nodes = NULL;
		tmp_Nodes = new c_Node*[p_Count];
		c_Node * return_NID = NULL;

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Nodes[cou_Index] = Base.get_Node_Ref_By_NID(p_Nodes[cou_Index]);
		}

		return_NID = Base.does_Upper_Tier_Connection_Exist(p_Count, tmp_Nodes);

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Nodes[cou_Index] = NULL;
		}

		if (tmp_Nodes != NULL) { delete[] tmp_Nodes; tmp_Nodes = NULL; }

		if (return_NID != NULL) { return return_NID->NID; }
		return 0;
	}


	//---==  does_Lower_Connection_Exist [NID] [NODES[]] [NODE_COUNT]  ==---//
    //Checks if the given node matches a dendrite on the right leg.
	bool does_Lower_Connection_Exist(uint64_t p_NID, int p_Count, uint64_t* p_Nodes)
	{
		c_Node** tmp_Nodes = NULL;
		tmp_Nodes = new c_Node*[p_Count];
		bool return_Result = 0;

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Nodes[cou_Index] = Base.get_Node_Ref_By_NID(p_Nodes[cou_Index]);
		}

		return_Result = Base.does_Lower_Connection_Exist(Base.get_Node_Ref_By_NID(p_NID), p_Count, tmp_Nodes);

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Nodes[cou_Index] = NULL;
		}

		if (tmp_Nodes != NULL) { delete[] tmp_Nodes; tmp_Nodes = NULL; }

		return return_Result;
	}



	//---==  bind_State [NID] [STATE_UINT]  ==---//
    //Binds a node to a quanta of data, the state of the input.
	void bind_State(uint64_t p_NID, uint64_t p_State)
	{
		Base.bind_State(Base.get_Node_Ref_By_NID(p_NID), p_State);
	}



	//---==  bp_O [NID]  ==---//
    //Initiates a backpropagation that outputs the pattern represented by this node.
	void bp_O(uint64_t p_NID)
	{
		Base.bp_O(Base.get_Node_Ref_By_NID(p_NID));
	}



	//---==  output_Node_Raw [NID]  ==---//
	//Outputs the ugly raw info dump for the node.
	void output_Node_Raw(uint64_t p_NID)
	{
		Base.output_Node_Raw(Base.get_Node_Ref_By_NID(p_NID));
	}


	//---==  output_Node_Raw [NID]  ==---//
	//Outputs the ugly raw info dump for the node.
	void output_Treetop_Node_Raw(std::string p_Construct)
	{
		Base.output_Treetop_Node_Raw(get_Construct_ID(p_Construct));
	}



	//---==  output_Node_Char [NID]  ==---//
	//Casts the node address to char() and outputs it.
	void output_Node_Char(uint64_t p_NID)
	{
		Base.output_Node_Char(Base.get_Node_Ref_By_NID(p_NID));
	}


	
  
  
//    ---==================---
//   ---====================---
//  ---==   Node_Network   ==---
//   ---====================---
//    ---==================---
  
	//---==  new_Node  ==---//
	//Creates a new node and adds it to the fractal tree.
	//Each node is stored as a link in a linked list.
	uint64_t new_Node()
	{
		return (Base.new_Node())->NID;
	}
  

	//---==  new_State_Node [Construct_ID] [STATE_DOUBLE]  ==---//
	//Creates a new node, then adds it to the state tree.
	//Assumes the construct is already registered so the index is valid.
	uint64_t new_State_Node(std::string p_Construct, uint64_t p_State)
	{
		return (Base.new_State_Node(get_Construct_ID(p_Construct), p_State))->NID;
	}
  

	//---==  create_Connections [TO_NID] [FROM_NIDS[]] [COUNT]  ==---//
	//Creates a connection between nodes.
	//p_To forms dendritic connections to p_From, and on p_From you have the axonic connections.
	void create_Connections(uint64_t p_To_NID, int p_Count, uint64_t* p_From_NID)
	{
		c_Node** tmp_From = NULL;
		tmp_From = new c_Node * [p_Count];

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_From[cou_Index] = Base.get_Node_Ref_By_NID(p_From_NID[cou_Index]);
		}

		Base.create_Connections(Base.get_Node_Ref_By_NID(p_To_NID), p_Count, tmp_From);

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_From[cou_Index] = NULL;
		}

		if (tmp_From != NULL) { delete[] tmp_From; tmp_From = NULL; }
	}
  

	//---==  does_Upper_Tier_Connection_Exist [LEGS] [LEG_COUNT]  ==---//
	//Checks if an upper tier node exists.
	uint64_t does_Upper_Tier_Connection_Exist_Network(int p_Count, uint64_t* p_Legs_NID)
	{
		c_Node** tmp_Legs = NULL;
		tmp_Legs = new c_Node * [p_Count];
		c_Node* tmp_Return_Node = NULL;

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Legs[cou_Index] = Base.get_Node_Ref_By_NID(p_Legs_NID[cou_Index]);
		}

		tmp_Return_Node = Base.does_Upper_Tier_Connection_Exist(p_Count, tmp_Legs);

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Legs[cou_Index] = NULL;
		}

		if (tmp_Legs != NULL) { delete[] tmp_Legs; tmp_Legs = NULL; }

		if (tmp_Return_Node != NULL) { return tmp_Return_Node->NID; }
		return NULL;
	}
  

	//---==  get_Upper_Tier_Node [LEGS] [LEG_COUNT]  ==---//
	//Gets an upper tier node based on the given legs. Will create it if not found.
	uint64_t get_Upper_Tier_Node(int p_Count, uint64_t* p_Legs_NID)
	{
		c_Node** tmp_Legs = NULL;
		tmp_Legs = new c_Node * [p_Count];
		c_Node* tmp_Return_Node = NULL;

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Legs[cou_Index] = Base.get_Node_Ref_By_NID(p_Legs_NID[cou_Index]);
		}

		tmp_Return_Node = Base.get_Upper_Tier_Node(p_Count, tmp_Legs);

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			tmp_Legs[cou_Index] = NULL;
		}

		if (tmp_Legs != NULL) { delete[] tmp_Legs; tmp_Legs = NULL; }

		if (tmp_Return_Node != NULL) { return tmp_Return_Node->NID; }
		return NULL;
	}

  
	//---==  does_State_Node_Exist [Construct_ID] [STATE]  ==---//
	//If a state node exists in the given construct index then return it.
	//Otherwise return NULL.
	//This assumes the [Index] is valid
	uint64_t does_State_Node_Exist(std::string p_Construct, uint64_t p_State)
	{
		c_Node* tmp_Node = NULL;
		tmp_Node = Base.does_State_Node_Exist(get_Construct_ID(p_Construct), p_State);

		if (tmp_Node != NULL) { return tmp_Node->NID; }
		return 0;
	}
  

	//---==  get_State_Node [Construct_ID] [STATE]  ==---//
	//Checks to see if a node in the given Construct is bound to the given state, if not the node is created.
	uint64_t get_State_Node(std::string p_Construct, uint64_t p_State)
	{
		c_Node* tmp_Node = NULL;
		tmp_Node = Base.get_State_Node(get_Construct_ID(p_Construct), p_State);

		if (tmp_Node != NULL) { return tmp_Node->NID; }
		return 0;
	}


	//---==  wipe_Network_Charges  ==---//
	//Wipes the current network charges, this is network wide.
	void wipe_Network_Charges()
	{
		Base.wipe_Network_Charges();
	}

	//---==  output_BP  ==---//
	//Iterates through every node and outputs their bp_O()
	void output_Backpropagated_Symbols(int p_Datatype)
	{
		Base.output_Backpropagated_Symbols(p_Datatype);
	}

	//---==  output_BP  ==---//
	//Iterates through every node and outputs their bp_O()
	void output_Backpropagated_Symbols_Float()
	{
		Base.output_Backpropagated_Symbols(2);
	}

  
	//---==  output_BP_NID [NID]  ==---//
	//Finds given NID and outputs the bp_O()
	void output_Backpropagated_Symbol_NID(uint64_t p_NID)
	{
		Base.output_Backpropagated_Symbol_NID(p_NID);
	}

	void output_Backpropagated_Symbol_NID_uint(uint64_t p_NID)
	{
		Base.output_Backpropagated_Symbol_NID_uint(p_NID);
	}
  
	//---==  output_Node_Network  ==---//
	//Outputs all of the nodes as raw.
	void output_Node_Network()
	{
		Base.output_Node_Network();
	}
  
	//---==  output_Network_Charges  ==---//
	//Outputs all of the charges as characters.
	void output_Network_Charges(std::string p_FName_Prefix = "")
	{
		Base.output_Network_Charges(p_FName_Prefix);
	}

	//    ---=========---
	//   ---===========---
	//  ---==   CAN   ==---
	//   ---===========---
	//    ---=========---

	void chrono_Shift(std::string p_Construct)
	{
		Base.chrono_Shift(get_Construct_ID(p_Construct));
	}

	void add_Chrono(std::string p_Construct, uint64_t p_Chrono)
	{
		Base.add_Chrono(get_Construct_ID(p_Construct), p_Chrono);
	}

	void pull_Chrono_From_Lower_Connection(std::string p_Construct)
	{
		Base.pull_Chrono_From_Lower_Connection(get_Construct_ID(p_Construct));
	}

	//---==  encode [Construct_ID]  ==---//
	//This encodes the p_Input data, if the nodes aren't found they are created, used for training.
	//Encodes the current input of the given Construct. How it does this is determined by what type of Construct it is.
	void encode(std::string p_Construct)
	{
		Base.encode(get_Construct_ID(p_Construct));
	}

	//This fills out the NULLCAN, but does not eval, and does not encode. Used when you want to check for an encoded symbol and get the Treetop.
	void check_Symbol(std::string p_Construct)
	{
		Base.check_Symbol(get_Construct_ID(p_Construct));
	}

	//    ---==  query [Construct_ID]  ==---
	//This sets up the scaffold as encode does, but it doesn't create nodes if they aren't found, they remain NULL in the scaffold, this we call a NULLCAN
	//Used for querying the network, you input, fill the NULLCAN, charge the network, then gather the outputs.
	//Suggested for use before encoding (if using learning mode and not locked_to_initial_training_mode) otherwise it will also find the current trace as the perfect match.
	//		Queries the network with the current input set for the given Construct.
		//Passes the values to an Construct to encode.
	void query(std::string p_Construct)
	{
		Base.query(get_Construct_ID(p_Construct));
	}

	//    ---==  query_spacial [Construct_ID]  ==---
	//		Queries the network with the current input set of the given Construct, but the input index determines what leg is charged. Meaning if an input at index [3] is charged then only upper tier nodes connected on axon hillock [3] will be charged.
		//Passes the values to an Construct to encode.
	void query_Spacial(std::string p_Construct)
	{
		Base.query(get_Construct_ID(p_Construct), 1);
	}

	//    ---==  query_given_index [Construct_ID] [INDEX]  ==---
	//		Queries the network with the current input set, however, every input node is charged on the given index. If INDEX is [3] then all nodes in the input set will be charging using axon hillock [3].
		//Passes the values to an Construct to encode.
	void query_Given_Index(std::string p_Construct, int p_Index)
	{
		Base.query(get_Construct_ID(p_Construct), 2, p_Index);
	}

	//    ---==  query_given_legs[Construct_ID] [LEG_COUNT] [LEG_0] [LEG_1] [...]  ==---
	void query_Given_Legs(std::string p_Construct, int p_Leg_Count, int* p_Legs)
	{
		Base.query(get_Construct_ID(p_Construct), 3, p_Leg_Count, p_Legs);
	}


	//---==  submit_Set [Construct_ID] [INPUT_UINT[]] [DEPTH]  ==---//
	//This allows for passing unordered sets of nodes
	void submit_Set(std::string p_Construct, int p_Depth, uint64_t* p_Input)
	{
		Base.submit_Set(get_Construct_ID(p_Construct), p_Depth, p_Input);
	}

	//Used for encoding.
	void round_Up_Input(std::string p_Construct)
	{
		Base.round_Up_Input(get_Construct_ID(p_Construct));
	}

	void round_Up_Given_Input(std::string p_Construct, int p_Input)
	{
		Base.round_Up_Given_Input(get_Construct_ID(p_Construct), p_Input);
	}

	//Used for eval, pulls the output from lower to upper.
	void pull_From_Lower_Connections(std::string p_Construct)
	{
		Base.pull_From_Lower_Connections(get_Construct_ID(p_Construct));
	}

	void pull_From_Lower_Connection(std::string p_Construct, int p_Index)
	{
		Base.pull_From_Lower_Connection(get_Construct_ID(p_Construct), p_Index);
	}

	//Iterates through every output trace in the given index of the given upper tier construct.
	void pull_From_Upper_Index(std::string p_Construct_To, std::string p_Construct_From, int p_Index)
	{
		Base.pull_From_Upper_Index(get_Construct_ID(p_Construct_To), get_Construct_ID(p_Construct_From), p_Index);
	}

	//---==  get_Treetop [Construct_ID]  ==---//
	//Gets the treetop node for a given Construct.
	//This returns the treetop node at a given index, for most structures this will be a single node, but for those like stiched-base networks with a treetop node count equal to the input node count then you can access them by index.
	//Gets the treetop node for a given Construct.
	uint64_t get_Treetop_NID(std::string p_Construct)
	{
		return Base.get_Treetop_NID(get_Construct_ID(p_Construct));
	}


	//---==  gather_Given_Trace [Construct_ID] [NID]  ==---//
	//Gets a single trace from a given node. Puts it into the output.
	void gather_Given_Trace(std::string p_Construct, uint64_t p_NID)
	{
		Base.gather_Given_Trace(get_Construct_ID(p_Construct), p_NID);
	}

	void write_Given_Pattern_As_Number(std::string p_Construct, uint64_t p_NID)
	{
		Base.write_Given_Pattern_As_Number(get_Construct_ID(p_Construct), p_NID);
	}


	//---==  gather_All_Traces [Construct]  ==---//
	//Gathers all the traces as it says.
	void gather_All_Traces(std::string p_Construct)
	{
		Base.gather_All_Traces(get_Construct_ID(p_Construct));
	}

	//---==  gather_All_Traces [Construct]  ==---//
	//Gathers all the traces as it says.
	void gather_All_Traces_uint(std::string p_Construct)
	{
		Base.gather_All_Traces_uint(get_Construct_ID(p_Construct));
	}


	//---==  reset_Input [Construct_ID]  ==---//
	//Wipe the input array.
	void reset_Input(std::string p_Construct)
	{
		Base.reset_Input(get_Construct_ID(p_Construct));
	}


	//---==  reset_Output [Construct_ID]  ==---//
	//Wipe the output array.
	void reset_Output(std::string p_Construct)
	{
		Base.reset_Output(get_Construct_ID(p_Construct));
	}


	//---==  set_State_Nodes_Index [Construct_ID] [INDEX]  ==---//
	//Sets the index for the state_Node_Tree in the c_Node_Network::State_Nodes[]
	void set_State_Nodes_Index(std::string p_Construct, int p_Index)
	{
		Base.set_State_Nodes_Index(get_Construct_ID(p_Construct), p_Index);
	}


	

	//---==  output_Scaffold [Construct_ID]  ==---//
	//Outputs the scaffold as addresses.
	void output_Scaffold(std::string p_Construct)
	{
		Base.output_Scaffold(get_Construct_ID(p_Construct));
	}


	//---==  output_Input [Construct_ID]  ==---//
	//		Outputs the input of the given Construct to the console.
	void output_Input(std::string p_Construct)
	{
		Base.output_Input(get_Construct_ID(p_Construct));
	}


	//---==  output_Input [Construct_ID]  ==---//
	//		Outputs the input of the given Construct to the console.
	void output_Input_uint(std::string p_Construct)
	{
		Base.output_Input_uint(get_Construct_ID(p_Construct));
	}


	//---==  output_Output [Construct_ID]  ==---//
	//The output trace set is output.
	void output_Output(std::string p_Construct)
	{
		Base.output_Output(get_Construct_ID(p_Construct));
	}



	//---==  output_Output [Construct_ID]  ==---//
	//The output trace set is output.
	void output_Output_uint(std::string p_Construct)
	{
		Base.output_Output_uint(get_Construct_ID(p_Construct));
	}
	
	void output_Output_Double(std::string p_Construct)
	{
		Base.output_Output_Double(get_Construct_ID(p_Construct));
	}
	
	
	void output_Output_Int(std::string p_Construct)
	{
		Base.output_Output_Int(get_Construct_ID(p_Construct));
	}


	//---==  output_Scaffold_Char [Construct_ID]  ==---//
	//Each address is typecast to a char to give a pseudo-unique look to each node. For monke brain.
	void output_Scaffold_Char(std::string p_Construct)
	{
		Base.output_Scaffold_Char(get_Construct_ID(p_Construct));
	}


	//---==  output_Scaffold_Char [Construct_ID]  ==---//
	//Each address is typecast to a char to give a pseudo-unique look to each node. For monke brain.
	void output_Scaffold_Symbols(std::string p_Construct)
	{
		Base.output_Scaffold_Symbols(get_Construct_ID(p_Construct));
	}

	void output_Scaffold_Symbols_uint(std::string p_Construct)
	{
		Base.output_Scaffold_Symbols_uint(get_Construct_ID(p_Construct));
	}

	void output_Scaffold_Symbols_Float(std::string p_Construct)
	{
		Base.output_Scaffold_Symbols_Float(get_Construct_ID(p_Construct));
	}





	//      ---====================---
	//     ---======================---
	//    ---========================---
	//   ---==========================---
	//  ---==   NT4 specific hooks   ==---
	//   ---==========================---
	//    ---========================---
	//     ---======================---
	//      ---====================---



	//    ---======================================---
	//   ---========================================---
	//  ---==   Used to register/connect new Constructs.   ==---
	//   ---========================================---
	//    ---======================================---

		//    ---==  register_Construct [Construct_TYPE] [Construct_NAME]  ==---
		//p_Type is the type of CAN to declare. 
		// "Many_To_One" - The I/O tier has every node connected to a single upper tier node.
	int register_Construct(std::string p_Type, std::string p_Construct_Name)
	{
		return Base.register_Construct(p_Type, p_Construct_Name);
	}

	void create_Construct_Connection(std::string p_From, std::string p_To)
	{
		Base.create_Construct_Connection(get_Construct_ID(p_From), get_Construct_ID(p_To));
	}

	void output_Construct_Connections(std::string p_Construct)
	{
		Base.output_Construct_Connections(get_Construct_ID(p_Construct));
	}

	//    ---=====================---
	//   ---=======================---
	//  ---==   Input handling.   ==---
	//   ---=======================---
	//    ---=====================---

	//    ---==  load_input  ==---
	int load_Input(std::string p_Construct)
	{
		return Base.load_Input(get_Construct_ID(p_Construct));
	}

	int load_Input_uint(std::string p_Construct)
	{
		return Base.load_Input_uint(get_Construct_ID(p_Construct));
	}

	//    ---==  set_input [Construct_ID] [INPUT_STRING]  ==---
		//Set the value to the passed 1D string of uint64_t
	void set_Input(std::string p_Construct, std::string p_Input)
	{
		Base.set_Input(get_Construct_ID(p_Construct), p_Input);
	}

	//    ---==  set_input_uint [Construct_ID] [ARRAY_DEPTH] [UINT_ARRAY]  ==---
		//Set the value to the passed 1D string of uint64_t
	void set_Input_uint(std::string p_Construct, int p_Depth, uint64_t* p_Input)
	{
		Base.set_Input_uint(get_Construct_ID(p_Construct), p_Depth, p_Input);
	}


	void set_Input_Charging_Mask(std::string p_Construct, std::vector<double> p_Input_Charging_Mask)
	{
		Base.set_Input_Charging_Mask(get_Construct_ID(p_Construct), p_Input_Charging_Mask);
	}


	//---==  set_Input [Construct_ID] [INPUT_UINT[]] [INPUT_DEPTH]  ==---//
	//Sets the input to the given uint64_t array.
	//The input array is 1D
	void set_Input(std::string p_Construct, int p_Input_Depth, uint64_t* p_Input)
	{
		Base.set_Input(get_Construct_ID(p_Construct), p_Input_Depth, p_Input);
	}



	//Sets the input to the given uint64_t 2d array.
	//The input array is 2D
	void set_2D_Input_uint(std::string p_Construct, int p_X_Depth, int p_Y_Depth, uint64_t** p_Input)
	{
		Base.set_2D_Input(get_Construct_ID(p_Construct), p_X_Depth, p_Y_Depth, p_Input);
	}


	//Sets the input to the given uint64_t 3d array.
	//The input array is 3D
	void set_3D_Input_uint(std::string p_Construct, int p_X_Depth, int p_Y_Depth, int p_Z_Depth, uint64_t*** p_Input)
	{
		Base.set_3D_Input(get_Construct_ID(p_Construct), p_X_Depth, p_Y_Depth, p_Z_Depth, p_Input);
	}



	//---==  set_Input_String [Construct_ID] [INPUT_STR]  ==---//
	//This is used for setting the input array to reflect a sequence of characters.
	void set_Input_String(std::string p_Construct, std::string p_Input)
	{
		Base.set_Input_String(get_Construct_ID(p_Construct), p_Input);
	}

	//    ---==========================================================================---
	//   ---============================================================================---
	//  ---==   Different ways of gathering nodes, individually, and the entire set.   ==---
	//   ---============================================================================---
	//    ---==========================================================================---

	//    ---==  gather_given_node [Construct_ID] [NID]  ==---
	//		Writes the given node's data down in the Construct_Output_Files[get_Construct_ID(p_Construct)] file.
	void gather_Given_Node(std::string p_Construct, uint64_t p_NID)
	{
		Base.gather_Given_Node(get_Construct_ID(p_Construct), p_NID);
	}

	//		Writes the given node's data down in the Construct_Output_Files[get_Construct_ID(p_Construct)] file.
	void gather_Given_Node_uint(std::string p_Construct, uint64_t p_NID)
	{
		Base.gather_Given_Node_uint(get_Construct_ID(p_Construct), p_NID);
	}

	//    ---==  gather_all_nodes [Construct_ID]  ==---
	//		This writes the entire network to the Construct_Output_Files[get_Construct_ID(p_Construct)] file. Note, the output patterns are treated as character.
		//It uses the passed Construct to output the nodes by putting it into the output of that Construct, then into the file.
	void gather_All_Nodes(std::string p_Construct)
	{
		Base.gather_All_Nodes(get_Construct_ID(p_Construct));
	}

	//    ---==  gather_all_nodes_uint [Construct_ID]  ==---
	//		This writes the entire network to the Construct_Output_Files[get_Construct_ID(p_Construct)] file. Note, the output patterns are treated as uint.
	void gather_All_Nodes_uint(std::string p_Construct)
	{
		Base.gather_All_Nodes_uint(get_Construct_ID(p_Construct));
	}


	//    ---==================================================================---
	//   ---====================================================================---
	//  ---==   The output of a given Construct is read into the output file.   ==---
	//   ---====================================================================---
	//    ---==================================================================---


	//    ---==  gather_output [Construct_ID]  ==---
	//		Takes every trace in the given Constructs output trace array and writes them to the Construct_Output_Files[get_Construct_ID(p_Construct)] file, note the output state patterns are treated as char.
	void gather_Output(std::string p_Construct)
	{
		Base.gather_Output(get_Construct_ID(p_Construct));
	}

	//    ---==  gather_output_uint [Construct_ID]  ==---
	//		Takes every trace in the given Constructs output trace array and writes them to the Construct_Output_Files[get_Construct_ID(p_Construct)] file, note the output state patterns are treated as uint.
	void gather_Output_uint(std::string p_Construct)
	{
		Base.gather_Output_uint(get_Construct_ID(p_Construct));
	}


	//    ---=============================---
	//   ---===============================---
	//  ---==   Treetop node gathering.   ==---
	//   ---===============================---
	//    ---=============================---

	//    ---==  gather_treetop_node [Construct_ID]  ==---
	//		This writes the current treetop node of the given Construct to the Construct_Output_Files[get_Construct_ID(p_Construct)] file. This does not erase the file.
	void gather_Treetop_Node(std::string p_Construct)
	{
		Base.gather_Treetop_Node(get_Construct_ID(p_Construct));
	}

	//    ---==  gather_treetop_node_uint [Construct_ID]  ==---
	//		This writes the current treetop node of the given Construct to the Construct_Output_Files[get_Construct_ID(p_Construct)] file. This does not erase the file.
	void gather_Treetop_Node_uint(std::string p_Construct)
	{
		Base.gather_Treetop_Node_uint(get_Construct_ID(p_Construct));
	}

	//    ---==  gather_treetop_NID [Construct_ID]  ==---
	//		This writes only the NID of the current treetop to the file. Does not erase the file.
	void gather_Treetop_NID(std::string p_Construct)
	{
		Base.gather_Treetop_NID(get_Construct_ID(p_Construct));
	}

	void write_Treetop_NID_To_Other_Input(std::string p_Construct_From, std::string p_Construct_To)
	{
		Base.write_Treetop_NID_To_Other_Input(get_Construct_ID(p_Construct_From), get_Construct_ID(p_Construct_To));
	}

	void gather_Treetops(std::string p_Construct)
	{
		Base.gather_Treetops(get_Construct_ID(p_Construct));
	}


	//    ---======================================================================---
	//   ---========================================================================---
	//  ---==   Output the Construct input, output, scaffolds, node network, etc.   ==---
	//   ---========================================================================---
	//    ---======================================================================---

	//    ---==  output_Constructs  ==---
	//		Outputs the Constructs currently registered.
	void output_Constructs()
	{
		Base.output_Constructs();
	}

	void output_Construct_Config(std::string p_Construct)
	{
		Base.output_Config(get_Construct_ID(p_Construct));
	}

	//      ---==================================---
	//     ---====================================---
	//    ---======================================---
	//   ---========================================---
	//  ---==   Generic commands for the engine.   ==---
	//   ---========================================---
	//    ---======================================---
	//     ---====================================---
	//      ---==================================---



	//    ---==  clear_output  ==---
	int clear_Output(std::string p_Construct)
	{
		return Base.clear_Output(get_Construct_ID(p_Construct));
	}


	//    ---==  output_newline  ==---
	int output_Newline(std::string p_Construct)
	{
		return Base.output_Newline(get_Construct_ID(p_Construct));
	}	
	
	int write_Text(std::string p_Construct, std::string p_Text)
	{
		return Base.write_Text(get_Construct_ID(p_Construct), p_Text);
	}


	void save(std::string p_FName)
	{
		Base.save(p_FName);
	}

	void load(std::string p_FName)
	{
		Base.load(p_FName);
	}
};