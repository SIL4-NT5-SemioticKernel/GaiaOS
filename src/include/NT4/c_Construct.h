/** \addtogroup Construct
 *  @{
 */

 /** \class c_Construct
	 \brief This class encapsulates and manipulates the Neural Network Engine directly while providing a public interface.

	 Inside the construct you'll find the raw C++. The interface brings highly granular control from the sub-classes to the surface. On this interface the other ones are built.

 */

//The construct encapsulates the node network, the CANs, I/O, granulation filter, uinterface, actuator interface, and I/O tables.
//The result is that after setting up the network the user can use it like a black box.
class c_Construct
{
public:

	//The shared node network.
	c_Node_Network Nodes;

	//The lookup tree for Construct names.
	c_Lookup_Tree Construct_Name_Tree;

	struct s_Construct
	{
		//The names of each Construct within the construct.
		std::string Name;

		//These are the files each Construct uses for input, defaults to "Input.ssv"
		std::string Input_File;

		//This file is is for the configuration file for the hyperparameters and other things
		std::string Config_File;

		//The file for each Construct to use as output.
		std::string Output_File;

		//This holds the CAN scaffold structures.
		//c_Base_CAN is a base class.
		c_Base_CAN* CAN;

		//This array keeps track of what type each CAN is.
		std::string CAN_Type;

		//This is the count of lower constructs.
		int Connection_Count;

		//These are the IDs of the lower constructs to connect to.
		int* Connections;

		s_Construct()
		{
			Name = "UNINITIALIZED";
			Input_File = "";
			Config_File = "";
			Output_File = "";
			CAN = NULL;
			CAN_Type = "UNINITIALIZED";
			Connection_Count = 0;
			Connections = NULL;
		}
	};

	s_Construct** Constructs;

	//For each Construct we have an index in the State_Tree[], CAN[], etc.
	int Construct_Count;

	c_Construct()
	{
		Construct_Count = 0; //Variables dependent on this value: Construct_Names, CAN
		Constructs = NULL;
	}

	//I need to refactor this jfc
	int add_Construct(std::string p_Construct_Name)
	{
		Construct_Name_Tree.search(p_Construct_Name);

		if (Construct_Name_Tree.flg_Foundit == 1) { std::cerr << "\n\n   V(O.o)/   Error: Construct [" << p_Construct_Name << "] already exists!"; return -1; }

		Construct_Name_Tree.set_Current_Data(Construct_Count);

		//Construct_Name_Tree.output_Tree();

		s_Construct** tmp_Constructs = NULL;

		tmp_Constructs = new s_Construct * [Construct_Count];

		for (int cou_Index = 0; cou_Index < Construct_Count; cou_Index++)
		{
			tmp_Constructs[cou_Index] = Constructs[cou_Index];
			Constructs[cou_Index] = NULL;
		}

		if (Constructs != NULL) { delete[] Constructs; }

		Constructs = new s_Construct * [Construct_Count + 1];


		for (int cou_Index = 0; cou_Index < Construct_Count; cou_Index++)
		{
			Constructs[cou_Index] = tmp_Constructs[cou_Index];
			tmp_Constructs[cou_Index] = NULL;
		}

		delete[] tmp_Constructs; tmp_Constructs = NULL;

		Constructs[Construct_Count] = new s_Construct;

		Constructs[Construct_Count]->Name = p_Construct_Name;

		//The reason the .Input and .Output are appended even though they are in folders named input and output this allows you to copy them into the same dir if you feel like it.
		Constructs[Construct_Count]->Input_File = ".\\Input\\" + p_Construct_Name + ".Input.ssv";
		Constructs[Construct_Count]->Config_File = ".\\Config\\" + p_Construct_Name + ".Config.ssv";
		Constructs[Construct_Count]->Output_File = ".\\Output\\" + p_Construct_Name + ".Output.ssv";

		std::ofstream tmp_IF(Constructs[Construct_Count]->Input_File, std::ios::app);
		tmp_IF.close();

		std::ofstream tmp_CF(Constructs[Construct_Count]->Config_File, std::ios::app);
		tmp_CF.close();

		std::ofstream tmp_OF(Constructs[Construct_Count]->Output_File, std::ios::app);
		tmp_OF.close();

		//The CAN is made later because there are different subtypes.
		Constructs[Construct_Count]->CAN = NULL;

		Constructs[Construct_Count]->CAN_Type = "NONE";

		Constructs[Construct_Count]->Connection_Count = 0;

		Constructs[Construct_Count]->Connections = NULL;

		Construct_Count++;

		//Return the index of the newly created Construct.
		return Construct_Count - 1;
	}

	int get_Construct_ID(std::string p_Construct_Name)
	{
		Construct_Name_Tree.search(p_Construct_Name);

		if (Construct_Name_Tree.flg_Foundit == 1) { return int(Construct_Name_Tree.get_Current_Data()); }
		std::cout << "\n   ^(o.O)V   ERROR: Construct ID [" << p_Construct_Name << "] not found!";
		return -1;
	}

	void chrono_Shift(int p_Construct)
	{
		Constructs[p_Construct]->CAN->chrono_Shift();
	}

	void add_Chrono(int p_Construct, uint64_t p_Chrono)
	{
		Constructs[p_Construct]->CAN->add_Chrono(p_Chrono);
	}

	void add_Connection_Index(int p_Construct)
	{
		int* tmp_Connections = new int[Constructs[p_Construct]->Connection_Count];

		if (Constructs[p_Construct]->Connection_Count > 0)
		{
			for (int cou_Index = 0; cou_Index < Constructs[p_Construct]->Connection_Count; cou_Index++)
			{
				tmp_Connections[cou_Index] = Constructs[p_Construct]->Connections[cou_Index];
			}

			delete[] Constructs[p_Construct]->Connections;
			Constructs[p_Construct]->Connections = NULL;
		}

		Constructs[p_Construct]->Connections = new int[Constructs[p_Construct]->Connection_Count + 1];

		for (int cou_Index = 0; cou_Index < Constructs[p_Construct]->Connection_Count; cou_Index++)
		{
			Constructs[p_Construct]->Connections[cou_Index] = tmp_Connections[cou_Index];
		}

		if (tmp_Connections != NULL) { delete[] tmp_Connections; tmp_Connections = NULL; }

		Constructs[p_Construct]->Connections[Constructs[p_Construct]->Connection_Count] = 0;

		Constructs[p_Construct]->Connection_Count++;
	}

	void create_Construct_Connection(int p_From, int p_To)
	{
		add_Connection_Index(p_To);

		Constructs[p_To]->Connections[Constructs[p_To]->Connection_Count - 1] = p_From;
	}

	void output_Construct_Connections(int p_Construct)
	{
		if (p_Construct == -1) { return; }
		std::cout << "\n\nLower Connections for " << Constructs[p_Construct]->Name;
		for (int cou_Index = 0; cou_Index < Constructs[p_Construct]->Connection_Count; cou_Index++)
		{
			int tmp_Con = Constructs[p_Construct]->Connections[cou_Index];
			std::cout << "\n [" << cou_Index << "] " << Constructs[tmp_Con]->Name;
		}
	}


	//==--- DIRECT_HOOK ---==//
	//This doesn't make sense to run through the API
	c_Node* get_Node_Ref_By_NID(uint64_t p_NID)
	{
		return Nodes.get_Node_Ref_By_NID(p_NID);
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



	//    ---==========---
	//   ---============---
	//  ---==   Node   ==---
	//   ---============---
	//    ---==========---

		//==--- DIRECT_HOOK ---==//
		//(0: State), (1: Branch), (2: Treetop), (3: State/Treetop)
	void set_Type(c_Node* p_Node, uint8_t p_Type)
	{
		if (p_Node != NULL)
		{
			p_Node->set_Type(p_Type);
		}
	}


	//==--- DIRECT_HOOK ---==//
	//Adds an axon to the axon list at the given index, if the index doesn't exist then exist it with resize_Axon_Hillocks()
	void add_Axon_Index(c_Node* p_Node, c_Node* p_Axon, int p_Index)
	{
		if (p_Node != NULL)
		{
			p_Node->add_Axon_Index(p_Axon, p_Index);
		}
	}


	//==--- DIRECT_HOOK ---==//
	//Sets the dendrites of the node.
	//This assumes the node has no dendrites yet, if it does you be dangling and jangling
	void set_Dendrites(c_Node* p_Node, int p_Count, c_Node** p_Dendrites)
	{
		if (p_Node != NULL)
		{
			p_Node->set_Dendrites(p_Dendrites, p_Count);
		}
	}


	//==--- DIRECT_HOOK ---==//
	//Searches the axons to see if an upper tier connection exists.
	//This is always called from the first leg, that is why we separate _F from normal.
	c_Node* does_Upper_Tier_Connection_Exist(int p_Count, c_Node** p_Nodes)
	{
		return Nodes.does_Upper_Tier_Connection_Exist(p_Nodes, p_Count);
	}


	//==--- DIRECT_HOOK ---==//
	//Checks if the given node matches a dendrite on the right leg.
	bool does_Lower_Connection_Exist(c_Node* p_Node, int p_Count, c_Node** p_Nodes)
	{
		if (p_Node != NULL)
		{
			return p_Node->does_Lower_Connection_Exist(p_Nodes, p_Count);
		}
		return 0;
	}



	//==--- DIRECT_HOOK ---==//
	//Binds a node to a quanta of data, the state of the input.
	void bind_State(c_Node* p_Node, uint64_t p_State)
	{
		if (p_Node != NULL)
		{
			p_Node->bind_State(p_State);
		}
	}



	//==--- DIRECT_HOOK ---==//
	//Initiates a backpropagation that outputs the pattern represented by this node.
	void bp_O(c_Node* p_Node)
	{
		if (p_Node != NULL)
		{
			p_Node->bp_O();
		}
	}



	//==--- DIRECT_HOOK ---==//
	//The CAN handles this for backpropagating a trace into a given CAN input. "gather_Given_Trace(uint64_t p_NID)"
	void bp_Trace_O(c_Node* p_Node, c_Linked_List_Handler* p_LL)
	{
		if ((p_Node != NULL) && (p_LL != NULL))
		{
			p_Node->bp_Trace_O(p_LL);
		}
	}



	//==--- DIRECT_HOOK ---==//
	//Outputs the ugly raw info dump for the node.
	void output_Node_Raw(c_Node* p_Node)
	{
		if (p_Node != NULL)
		{
			p_Node->output_Node_Raw();
		}
	}



	//==--- DIRECT_HOOK ---==//
	//Outputs the ugly raw info dump for the node.
	void output_Treetop_Node_Raw(int p_Construct)
	{
		if ((Constructs[p_Construct]->CAN->get_Treetop()) != NULL)
		{
			(Constructs[p_Construct]->CAN->get_Treetop())->output_Node_Raw();
		}
	}



	//==--- DIRECT_HOOK ---==//
	//Casts the node address to char() and outputs it.
	void output_Node_Char(c_Node* p_Node)
	{
		if (p_Node != NULL)
		{
			p_Node->output_Node_Char();
		}
	}





	//    ---==================---
	//   ---====================---
	//  ---==   Node_Network   ==---
	//   ---====================---
	//    ---==================---

		//==--- DIRECT_HOOK ---==//
		//Creates a new node and adds it to the fractal tree.
		//Each node is stored as a link in a linked list.
	c_Node* new_Node()
	{
		return Nodes.new_Node();
	}


	//==--- DIRECT_HOOK ---==//
	//Creates a new node, then adds it to the state tree.
	//Assumes the construct is already registered so the index is valid.
	c_Node* new_State_Node(int p_Construct, uint64_t p_State)
	{
		return Nodes.new_State_Node(p_Construct, p_State);
	}


	//==--- DIRECT_HOOK ---==//
	//Creates a connection between nodes.
	//p_To forms dendritic connections to p_From, and on p_From you have the axonic connections.
	void create_Connections(c_Node* p_To, int p_Count, c_Node** p_From)
	{
		Nodes.create_Connections(p_To, p_From, p_Count);
	}


	//==--- DIRECT_HOOK ---==//
	//Checks if an upper tier node exists.
	c_Node* does_Upper_Tier_Connection_Exist_Network(int p_Count, c_Node** p_Legs)
	{
		return Nodes.does_Upper_Tier_Connection_Exist(p_Legs, p_Count);
	}


	//==--- DIRECT_HOOK ---==//
	//Gets an upper tier node based on the given legs. Will create it if not found and give it the type 1.
	c_Node* get_Upper_Tier_Node(int p_Count, c_Node** p_Legs)
	{
		return Nodes.get_Upper_Tier_Node(p_Legs, p_Count, 1);
	}


	//==--- DIRECT_HOOK ---==//
	//If a state node exists in the given construct index then return it.
	//Otherwise return NULL.
	//This assumes the [Index] is valid
	c_Node* does_State_Node_Exist(int p_Construct, uint64_t p_State)
	{
		return Nodes.does_State_Node_Exist(p_Construct, p_State);
	}


	//==--- DIRECT_HOOK ---==//
	//Checks to see if a node in the given Construct is bound to the given state, if not the node is created.
	c_Node* get_State_Node(int p_Construct, uint64_t p_State)
	{
		return Nodes.get_State_Node(p_Construct, p_State);
	}


	//==--- DIRECT_HOOK ---==//
	//Iterates through every node and outputs their bp_O()
	void output_Backpropagated_Symbols(int p_Datatype = 0)
	{
		Nodes.output_BP(p_Datatype);
	}


	//==--- DIRECT_HOOK ---==//
	//Finds given NID and outputs the bp_O()
	void output_Backpropagated_Symbol_NID(uint64_t p_NID)
	{
		Nodes.output_BP_NID(p_NID);
	}


	void output_Backpropagated_Symbol_NID_uint(uint64_t p_NID)
	{
		Nodes.output_Symbol_uint(p_NID);
	}


	//---==  DIRECT_HOOK  ==---//
	//Outputs all of the nodes as raw.
	void output_Node_Network()
	{
		Nodes.output_Raw();
	}



	//    ---=========---
	//   ---===========---
	//  ---==   CAN   ==---
	//   ---===========---
	//    ---=========---



	//==--- DIRECT_HOOK ---==//
	//This encodes the p_Input data, if the nodes aren't found they are created, used for training.
	void encode(int p_Construct)
	{
		Constructs[p_Construct]->CAN->encode();
	}
	

	void check_Symbol(int p_Construct)
	{
		Constructs[p_Construct]->CAN->check_Symbol();
	}

	//==--- DIRECT_HOOK ---==//
	//This sets up the scaffold as encode does, but it doesn't create nodes if they aren't found, they remain NULL in the scaffold, this we call a NULLCAN
	//Used for querying the network, you input, fill the NULLCAN, charge the network, then gather the outputs.
	//Suggested for use before encoding (if using learning mode and not locked_to_initial_training_mode) otherwise it will also find the current trace as the perfect match.
	void query(int p_Construct, int p_Charging_Style = -1, int p_Leg = 0, int* p_Legs = NULL)
	{
		if (p_Charging_Style == 4) { wipe_Network_Charges(); output_Network_Charges(); }
		
		Constructs[p_Construct]->CAN->query(p_Charging_Style, p_Leg, p_Legs);

		if (p_Charging_Style == 4) { output_Network_Charges(); }
	}


	//==--- DIRECT_HOOK ---==//
	//This allows for passing unordered sets of nodes
	void submit_Set(int p_Construct, int p_Depth, uint64_t* p_Input)
	{
		Constructs[p_Construct]->CAN->submit_Set(p_Input, p_Depth);
	}


	//==--- DIRECT_HOOK ---==//
	//Gets the treetop node for a given Construct.
	//This doesn't make sense to create an API for.
	//This returns the treetop node at a given index, for most structures this will be a single node, but for those like stiched-base networks with a treetop node count equal to the input node count then you can access them by index.
	c_Node* get_Treetop(int p_Construct)
	{
		return (Constructs[p_Construct]->CAN->get_Treetop());
	}


	//==--- DIRECT_HOOK ---==//
	//Gets the treetop node for a given Construct.
	uint64_t get_Treetop_NID(int p_Construct)
	{
		if (Constructs[p_Construct]->CAN->get_Treetop() != NULL)
		{
			return (Constructs[p_Construct]->CAN->get_Treetop())->NID;
		}
		return 0;
	}


	//==--- DIRECT_HOOK ---==//
	//Gets the treetop node for a given Construct.
	uint64_t get_Treetop_NID_At_Given_Index(int p_Construct, int p_Index)
	{
		if (Constructs[p_Construct]->CAN->get_Treetop(p_Index) != NULL)
		{
			return (Constructs[p_Construct]->CAN->get_Treetop(p_Index))->NID;
		}
		return 0;
	}


	//==--- DIRECT_HOOK ---==//
	//Gets a single trace from a given node. Puts it into the output.
	void gather_Given_Trace(int p_Construct, uint64_t p_NID)
	{
		Constructs[p_Construct]->CAN->gather_Given_Trace(p_NID);

		gather_Output(p_Construct);
	}


	//==--- DIRECT_HOOK ---==//
	//Gets a single trace from a given node. Puts it into the output.
	void write_Given_Pattern_As_Number(int p_Construct, uint64_t p_NID)
	{
		Constructs[p_Construct]->CAN->gather_Given_Trace(p_NID);

		gather_Output_uint(p_Construct);
	}


	//==--- DIRECT_HOOK ---==//
	//Gets a single trace from a given node. Puts it into the output.
	void gather_Given_Trace_uint(int p_Construct, uint64_t p_NID)
	{
		Constructs[p_Construct]->CAN->gather_Given_Trace(p_NID);

		gather_Output_uint(p_Construct);
	}


	//==--- DIRECT_HOOK ---==//
	//Gathers all the traces as it says.
	void gather_All_Traces(int p_Construct)
	{
		Constructs[p_Construct]->CAN->gather_All_Traces();

		gather_Output(p_Construct);
	}  


	//==--- DIRECT_HOOK ---==//
	//Gathers all the traces as it says.
	void gather_All_Traces_uint(int p_Construct)
	{
		Constructs[p_Construct]->CAN->gather_All_Traces();

		gather_Output_uint(p_Construct);
	}


	//==--- DIRECT_HOOK ---==//
	//Wipe the input array.
	void reset_Input(int p_Construct)
	{
		Constructs[p_Construct]->CAN->reset_Input();
	}



	//==--- DIRECT_HOOK ---==//
	//Wipe the input array.
	void reset_Output(int p_Construct)
	{
		Constructs[p_Construct]->CAN->reset_Output();
	}


	//==--- DIRECT_HOOK ---==//
	//Associate the CAN with a network from which to draw nodes.
	//This doesn't make sense to wrap in the API.
	void set_NNet(int p_Construct, c_Node_Network* p_NNet)
	{
		Constructs[p_Construct]->CAN->set_NNet(p_NNet);
	}


	//==--- DIRECT_HOOK ---==//
	//Sets the index for the state_Node_Tree in the c_Node_Network::State_Nodes[]
	void set_State_Nodes_Index(int p_Construct, int p_Index)
	{
		Constructs[p_Construct]->CAN->set_State_Nodes_Index(p_Index);
	}


	//==--- DIRECT_HOOK ---==//
	//Sets the input to the given uint64_t array.
	//The input array is 1D.
	void set_Input(int p_Construct, int p_Input_Depth, uint64_t* p_Input)
	{
		Constructs[p_Construct]->CAN->set_Input(p_Input, p_Input_Depth);
	}

	//==--- DIRECT_HOOK ---==//
	//Sets the input to the given uint64_t array.
	//The input array is 2D.
	void set_2D_Input(int p_Construct, int p_X_Depth, int p_Y_Depth, uint64_t** p_Input)
	{
		Constructs[p_Construct]->CAN->set_2D_Input(p_Input, p_X_Depth, p_Y_Depth);
	}

	//==--- DIRECT_HOOK ---==//
	//Sets the input to the given uint64_t array.
	//The input array is 3D.
	void set_3D_Input(int p_Construct, int p_X_Depth, int p_Y_Depth, int p_Z_Depth, uint64_t*** p_Input)
	{
		Constructs[p_Construct]->CAN->set_3D_Input(p_Input, p_X_Depth, p_Y_Depth, p_Z_Depth);
	}


	//==--- DIRECT_HOOK ---==//
	//This is used for setting the input array to reflect a sequence of characters.
	void set_Input_String(int p_Construct, std::string p_Input)
	{
		Constructs[p_Construct]->CAN->set_Input_String(p_Input);
	}


	//==--- DIRECT_HOOK ---==//
	//Outputs the scaffold as addresses.
	void output_Scaffold(int p_Construct)
	{
		Constructs[p_Construct]->CAN->output_Scaffold();
	}


	//==--- DIRECT_HOOK ---==//	
	//==--- CLI_HOOK ---==//	
	//    ---==  output_input [Construct_ID]  ==---
	//		Outputs the input of the given Construct to the console.
	//Outputs the input for the Construct.
	void output_Input(int p_Construct)
	{
		Constructs[p_Construct]->CAN->output_Input();
	}


	//==--- DIRECT_HOOK ---==//	
	//==--- CLI_HOOK ---==//	
	//    ---==  output_input [Construct_ID]  ==---
	//		Outputs the input of the given Construct to the console.
	//Outputs the input for the Construct.
	void output_Input_uint(int p_Construct)
	{
		Constructs[p_Construct]->CAN->output_Input(1);
	}


	//==--- DIRECT_HOOK ---==//
	//The output trace set is output.
	void output_Output(int p_Construct)
	{
		Constructs[p_Construct]->CAN->output_Output(0);
	}


	//==--- DIRECT_HOOK ---==//
	//The output trace set is output.
	void output_Output_uint(int p_Construct)
	{
		Constructs[p_Construct]->CAN->output_Output(1);
	}


	//==--- DIRECT_HOOK ---==//
	//The output trace set is output.
	void output_Output_Double(int p_Construct)
	{
		Constructs[p_Construct]->CAN->output_Output(2);
	}


	//==--- DIRECT_HOOK ---==//
	//The output trace set is output.
	void output_Output_Int(int p_Construct)
	{
		Constructs[p_Construct]->CAN->output_Output(3);
	}


	//==--- DIRECT_HOOK ---==//
	//Each address is typecast to a char to give a pseudo-unique look to each node. For monke brain.
	void output_Scaffold_Char(int p_Construct)
	{
		Constructs[p_Construct]->CAN->output_Scaffold_Char();
	}

	//==--- DIRECT_HOOK ---==//
	//Each address is typecast to a char to give a pseudo-unique look to each node. For monke brain.
	//Type: 0 = char, 1 = uint64_t
	void output_Scaffold_Symbols(int p_Construct, int p_Type = 0)
	{
		Constructs[p_Construct]->CAN->output_Scaffold_Symbols(p_Type);
	}

	void output_Scaffold_Symbols_uint(int p_Construct)
	{
		output_Scaffold_Symbols(p_Construct, 1);
	}

	void output_Scaffold_Symbols_Float(int p_Construct)
	{
		output_Scaffold_Symbols(p_Construct, 2);
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


	//Used to wipe the charges of the network.
	void wipe_Network_Charges()
	{
		c_Node* tmp_Node = NULL;
		tmp_Node = Nodes.Root;

		while (tmp_Node != NULL)
		{
			tmp_Node->Charge = 0;

			tmp_Node = tmp_Node->Next;
		}
	}

	void output_Network_Charges(std::string p_FName_Prefix = "")
	{
		std::cout << "\n Charges:";
		c_Node* tmp_Node = NULL;
		tmp_Node = Nodes.Root;
		
		std::string tmp_OFName = "";

		if (p_FName_Prefix != "")
		{
			tmp_OFName = ".\\Testing\\" + p_FName_Prefix + "node_Charge_Output.ssv";
		}
		else
		{
			tmp_OFName = ".\\Testing\\node_Charge_Output.ssv";
		}

		std::ofstream OF;
		//std::ofstream OFXY;
		//std::ofstream OFRC;
		OF.open(tmp_OFName, std::ios::app);
		//OFXY.open("./Testing/Node_Network_Output/node_Charge_Output_XY.dat", std::ios::app);
		//OFRC.open("./Testing/Node_Network_Output/node_RC_Output.dat", std::ios::app);

		int tmp_Cur = 0;
		char tmp_Char = ' ';
		
		int tmp_Top_Tier = 0;

		tmp_Node = NULL;
		tmp_Node = Nodes.Root;
		
		std::cout << "\n Finding Top_Tier & Widest_Tier...";
		while (tmp_Node != NULL)
		{
			if (tmp_Top_Tier <= tmp_Node->Tier)
			{
				tmp_Top_Tier = tmp_Node->Tier + 1;
			}

			tmp_Node = tmp_Node->Next;
		}

		std::cout << "Top_Tier: " << tmp_Top_Tier;

		std::vector<int> Skipdex;
		Skipdex.resize(tmp_Top_Tier);

		float tmp_Skipval = 0.0;
		float tmp_Skipcur = 0.0;

		std::cout << "\n SkipDex Building...";

		for (int cou_T = 0; cou_T < tmp_Top_Tier; cou_T++)
		{
			Skipdex[cou_T] = 0;
		}

		tmp_Node = NULL;
		tmp_Node = Nodes.Root;

		while (tmp_Node != NULL)
		{
			Skipdex[tmp_Node->Tier]++;

			if (Nodes.Fat_Tier < Skipdex[tmp_Node->Tier])
			{ 
				Nodes.Fat_Tier = Skipdex[tmp_Node->Tier];
			}

			tmp_Node = tmp_Node->Next;

		}

		std::cout << "\n Constructing XY Mapping...";

		std::vector<int> Skipdex_Cur;
		std::vector<int> Skipdex_Val;

		std::vector<std::vector<std::string>> Skipdex_Out;
		//std::vector<std::vector<std::string>> Skipdex_RC_Out;
		//std::vector<std::vector<std::string>> Skipdex_XY_Out;

		Skipdex_Cur.resize(tmp_Top_Tier);
		Skipdex_Val.resize(tmp_Top_Tier);

		Skipdex_Out.resize(tmp_Top_Tier);
		//Skipdex_RC_Out.resize(tmp_Top_Tier);
		//Skipdex_XY_Out.resize(tmp_Top_Tier);

		std::cout << "Allocating Memory...";
		for (int cou_T = 0; cou_T < tmp_Top_Tier; cou_T++)
		{
			Skipdex_Cur[cou_T] = 0;
			Skipdex_Val[cou_T] = Nodes.Fat_Tier / Skipdex[cou_T];

			Skipdex_Out[cou_T].resize(Nodes.Fat_Tier);
			//Skipdex_RC_Out[cou_T].resize(Nodes.Fat_Tier);
			//Skipdex_XY_Out[cou_T].resize(Nodes.Fat_Tier);

			/*
			for (int cou_FT = 0; cou_FT < Nodes.Fat_Tier; cou_FT++)
			{
				Skipdex_Out[cou_T][cou_FT] = " 0";
				Skipdex_RC_Out[cou_T][cou_FT] = " 0";
				Skipdex_XY_Out[cou_T][cou_FT] = "\n(" + std::to_string(cou_FT) + ", " + std::to_string(cou_T) + ") 0";
			}
			*/
		}

		tmp_Node = NULL;
		tmp_Node = Nodes.Root;

		std::cout << "Calculating XY...";
		while (tmp_Node != NULL)
		{
			Skipdex_Out[tmp_Node->Tier][int(Skipdex_Cur[tmp_Node->Tier])] = " " + std::to_string(tmp_Node->Charge);
			//Skipdex_RC_Out[tmp_Node->Tier][int(Skipdex_Cur[tmp_Node->Tier])] = " " + std::to_string(tmp_Node->RC);
			//Skipdex_XY_Out[tmp_Node->Tier][int(Skipdex_Cur[tmp_Node->Tier])] = "\n(" + std::to_string(int(Skipdex_Cur[tmp_Node->Tier])) + ", " + std::to_string(tmp_Node->Tier) + ") " + std::to_string(tmp_Node->NID);

			tmp_Node->Index = int(Skipdex_Cur[tmp_Node->Tier]);

			Skipdex_Cur[tmp_Node->Tier] += Skipdex_Val[tmp_Node->Tier];

			if (Skipdex_Cur[tmp_Node->Tier] >= Nodes.Fat_Tier) { Skipdex_Cur[tmp_Node->Tier] = Nodes.Fat_Tier - 1; }

			//Skipdex_Cur[tmp_Node->Tier]++;

			tmp_Node = tmp_Node->Next;
		}

		std::cout << "Writing Files...";
		/*
		for (int cou_I = 0; cou_I < Nodes.Fat_Tier; cou_I++)
		{
			OF << " 0";
			OFRC << " 0";
		}
		*/
		for (int cou_T = 0; cou_T < tmp_Top_Tier; cou_T++)
		{
			OF << "\n";
			//OFXY << "\n";
			//OFRC << "\n";

			OF << " " << cou_T << " ";
			//OFRC << " " << cou_T << " ";

			for (int cou_I = 0; cou_I < Nodes.Fat_Tier; cou_I++)
			{
				if (Skipdex_Out[cou_T][cou_I] == "") 
				{
					OF << " 0";
					//OFRC << " 0";
					//OFXY << "\n(" + std::to_string(cou_I) + ", " + std::to_string(cou_T) + ") 0";
					continue;
				}

				OF << Skipdex_Out[cou_T][cou_I];
				//OFRC << Skipdex_RC_Out[cou_T][cou_I];
				//OFXY << Skipdex_XY_Out[cou_T][cou_I];

			}
		}
		/*
		OF << "\n";
		OFXY << "\n";
		OFRC << "\n";
		for (int cou_I = 0; cou_I < Nodes.Fat_Tier; cou_I++)
		{
			OF << " 0";
			OFRC << " 0";
		}
		OF << "\n";
		OFXY << "\n";
		OFRC << "\n";*/

		OF.close();
		//OFRC.close();
		//OFXY.close();

		/*
		tmp_Node = NULL;
		tmp_Node = Nodes.Root;

		for (int cou_T = 0; cou_T < tmp_Top_Tier; cou_T++)
		{
			tmp_Node = NULL;
			tmp_Node = Nodes.Root;
			
			OF << "\n";
			OFXY << "\n";
			OFRC << "\n";
			//std::cout << "\n";

			tmp_Skipcur = 0.0;
			tmp_Skipval = Nodes.Fat_Tier / Skipdex[cou_T];

			std::vector<std::string> tmp_Out;
			std::vector<std::string> tmp_RC_Out;
			std::vector<std::string> tmp_XY_Out;
			tmp_Out.resize(Nodes.Fat_Tier);
			tmp_RC_Out.resize(Nodes.Fat_Tier);
			tmp_XY_Out.resize(Nodes.Fat_Tier);
			
			for (int cou_FT = 0; cou_FT < Nodes.Fat_Tier; cou_FT++)
			{
				tmp_Out[cou_FT] = " 0";
				tmp_RC_Out[cou_FT] = " 0";
				tmp_XY_Out[cou_FT] = "\n(" + std::to_string(cou_FT) + ", " + std::to_string(cou_T) + ") 0";
			}



			tmp_Cur = 0;


			while (tmp_Node != NULL)
			{
				if (tmp_Node->Tier == cou_T)
				{
					tmp_Out[int(tmp_Skipcur)] = " " + std::to_string(tmp_Node->Charge);

					tmp_RC_Out[int(tmp_Skipcur)] = " " + std::to_string(tmp_Node->RC);

					tmp_XY_Out[int(tmp_Skipcur)] = "\n(" + std::to_string(int(tmp_Skipcur)) + ", " + std::to_string(tmp_Node->Tier) + ") " + std::to_string(tmp_Node->NID);

					tmp_Node->Index = int(tmp_Skipcur);

					tmp_Skipcur += tmp_Skipval;

					if (tmp_Skipcur >= Nodes.Fat_Tier) { (tmp_Skipcur = Nodes.Fat_Tier - 1); }

					tmp_Cur++;
				}

				tmp_Node = tmp_Node->Next;
			}

			OF << " " << cou_T << " ";
			OFRC << " " << cou_T << " ";

			for (int cou_FT = 0; cou_FT < Nodes.Fat_Tier; cou_FT++)
			{
				OF << tmp_Out[cou_FT];
				OFRC << tmp_RC_Out[cou_FT];
				OFXY << tmp_XY_Out[cou_FT];
			}
		}
		OF.close();
		OFRC.close();
		OFXY.close();
		*/
		std::cout << "Complete...";
	}


	//    ---======================================---
	//   ---========================================---
	//  ---==   Used to register new Constructs.   ==---
	//   ---========================================---
	//    ---======================================---

		//    ---==  register_Construct [Construct_TYPE] [Construct_NAME]  ==---
		//p_Type is the type of CAN to declare. 
		// "Many_To_One" - The I/O tier has every node connected to a single upper tier node.
	int register_Construct(std::string p_Type, std::string p_Construct_Name)
	{
		int tmp_Construct_ID = add_Construct(p_Construct_Name);

		if (tmp_Construct_ID == -1) { return -1; }

		bool flg_Made_It = false;

		if (p_Type == "Many_To_One")
		{
			Constructs[tmp_Construct_ID]->CAN = new c_CAN_Many_To_One;

			//Make sure we set the node network for the CAN.
			Constructs[tmp_Construct_ID]->CAN->set_NNet(&Nodes);

			Constructs[tmp_Construct_ID]->CAN->State_Nodes_Index = Nodes.register_New_Construct();

			Constructs[tmp_Construct_ID]->CAN_Type = "Many_To_One";

			flg_Made_It = true;
		}
		if (p_Type == "1D_Pyramid")
		{
			Constructs[tmp_Construct_ID]->CAN = new c_CAN_1D_Pyramid;

			//Make sure we set the node network for the CAN.
			Constructs[tmp_Construct_ID]->CAN->set_NNet(&Nodes);

			Constructs[tmp_Construct_ID]->CAN->State_Nodes_Index = Nodes.register_New_Construct();

			Constructs[tmp_Construct_ID]->CAN_Type = "1D_Pyramid";

			flg_Made_It = true;
		}
		if (p_Type == "2D_Pyramid")
		{
			Constructs[tmp_Construct_ID]->CAN = new c_CAN_2D_Pyramid;

			//Make sure we set the node network for the CAN.
			Constructs[tmp_Construct_ID]->CAN->set_NNet(&Nodes);

			Constructs[tmp_Construct_ID]->CAN->State_Nodes_Index = Nodes.register_New_Construct();

			Constructs[tmp_Construct_ID]->CAN_Type = "2D_Pyramid";

			flg_Made_It = true;
		}
		if (p_Type == "3D_Pyramid")
		{
			Constructs[tmp_Construct_ID]->CAN = new c_CAN_3D_Pyramid;

			//Make sure we set the node network for the CAN.
			Constructs[tmp_Construct_ID]->CAN->set_NNet(&Nodes);

			Constructs[tmp_Construct_ID]->CAN->State_Nodes_Index = Nodes.register_New_Construct();

			Constructs[tmp_Construct_ID]->CAN_Type = "3D_Pyramid";

			flg_Made_It = true;
		}

		if (!flg_Made_It)
		{
			std::cerr << "\n\n /(>.>)/ WARNING: " << p_Type << " type construct not recognized! Construct [" << tmp_Construct_ID << "] defaulted to Many_To_One!\n";

			Constructs[tmp_Construct_ID]->CAN = new c_CAN_Many_To_One;

			//Make sure we set the node network for the CAN.
			Constructs[tmp_Construct_ID]->CAN->set_NNet(&Nodes);

			Constructs[tmp_Construct_ID]->CAN->State_Nodes_Index = Nodes.register_New_Construct();

			Constructs[tmp_Construct_ID]->CAN_Type = "Many_To_One";
		}

		save_Config(tmp_Construct_ID);

		//output_Constructs();

		return tmp_Construct_ID;
	}




	//    ---=====================---
	//   ---=======================---
	//  ---==   Input handling.   ==---
	//   ---=======================---
	//    ---=====================---

//    ---==  load_input  ==---
	int load_Input(int p_Construct)
	{
		std::ifstream InputFile(Constructs[p_Construct]->Input_File);

		std::string tmp_Input_Full = "";
		std::string tmp_In = "";
		int tmp_Count = 0;

		if (InputFile.is_open())
		{
			while (!InputFile.eof())
			{
				tmp_In = "";
				InputFile >> tmp_In;
				if (tmp_In == "") { continue; }
				std::cout << "\n - [ " << tmp_Count << " ]: " << tmp_In;
				tmp_Count++;

				if (tmp_Input_Full != "") { tmp_Input_Full = tmp_Input_Full + " " + tmp_In; }
				if (tmp_Input_Full == "") { tmp_Input_Full = tmp_In; }

				//Neuralman.output_Input();
			}

			std::cout << "\n Input.ssv contents: " << tmp_Input_Full << "\n";

			//set_Input_1D_string(int p_Construct, std::string p_Input)

			set_Input(p_Construct, tmp_Input_Full);

			return 1;
		}
		else
		{
			std::cerr << "\n Unable to open Input.ssv for set_Input_1D_string ...\n";

			return 0;
		}

		return 1;
	}

	//    ---==  load_input  ==---
	int load_Input_uint(int p_Construct)
	{
		std::ifstream InputFile(Constructs[p_Construct]->Input_File);

		int tmp_Count = 0;
		int tmp_Current = 0;
		uint64_t* tmp_Input = NULL;

		if (InputFile.is_open())
		{
			//Get the count:
			if (!InputFile.eof())
			{
				InputFile >> tmp_Count;
			}

			std::cout << "\n Count: " << tmp_Count;

			if (tmp_Count > 0)
			{
				tmp_Input = new uint64_t[tmp_Count];

				for (int cou_Index = 0; cou_Index < tmp_Count; cou_Index++)
				{
					tmp_Input[cou_Index] = 0;
				}

				while (!InputFile.eof())
				{
					InputFile >> tmp_Input[tmp_Current];

					std::cout << "\n - [ " << tmp_Current << " ]: " << tmp_Input[tmp_Current];
					tmp_Current++;
				}
			}

			set_Input_uint(p_Construct, tmp_Count, tmp_Input);

			if (tmp_Input != NULL) { delete[] tmp_Input; tmp_Input = NULL; }

			return 1;
		}
		else
		{
			std::cerr << "\n Unable to open Input.ssv for set_Input_1D_string ...\n";

			return 0;
		}

		return 1;
	}

	void round_Up_Input(int p_Construct)
	{
		uint64_t* tmp_Input = new uint64_t[Constructs[p_Construct]->Connection_Count];

		for (int cou_C = 0; cou_C < Constructs[p_Construct]->Connection_Count; cou_C++)
		{
			tmp_Input[cou_C] = 0;
			tmp_Input[cou_C] = get_Treetop_NID(Constructs[p_Construct]->Connections[cou_C]);
		}

		set_Input_uint(p_Construct, Constructs[p_Construct]->Connection_Count, tmp_Input);

		if (tmp_Input != NULL) { delete[] tmp_Input; }
	}

	void round_Up_Given_Input(int p_Construct, int p_Input)
	{
		uint64_t tmp_Input = get_Treetop_NID(Constructs[p_Construct]->Connections[p_Input]);
		set_Input_uint(p_Construct, 1, &tmp_Input);
	}

	int get_Output_Depth(int p_Construct)
	{
		return int(Constructs[p_Construct]->CAN->Output.size());
	}

	std::string get_Output_Pattern(int p_Construct, int p_Index)
	{
		return (Constructs[p_Construct]->CAN->get_Output(p_Index))->get_Pattern();
	}

	std::vector<uint64_t> get_Output_Pattern_uint(int p_Construct, int p_Index)
	{
		return (Constructs[p_Construct]->CAN->get_Output(p_Index))->get_Pattern_uint();
	}

	uint64_t get_Output_Primitive_uint(int p_Construct, int p_Index, int p_Primitive)
	{
		return (Constructs[p_Construct]->CAN->get_Output(p_Index))->get_Primitive_uint(p_Primitive);
	}

	int get_Output_Pattern_Size(int p_Construct, int p_Index)
	{
		return (Constructs[p_Construct]->CAN->get_Output(p_Index))->get_Output_Pattern_Size();
	}

	float get_Output_Charge(int p_Construct, int p_Index)
	{
		return (Constructs[p_Construct]->CAN->get_Output(p_Index))->get_Charge();
	}

	uint64_t get_Output_Treetop_NID(int p_Construct, int p_Index)
	{
		return (Constructs[p_Construct]->CAN->get_Output(p_Index))->get_Treetop_NID();
	}

	float get_Output_RC(int p_Construct, int p_Index)
	{
		return (Constructs[p_Construct]->CAN->get_Output(p_Index))->get_RC();
	}

	int get_Construct_Count()
	{
		return Construct_Count;
	}

	//Used for eval, pulls the output from lower to upper.
	void pull_From_Lower_Connections(int p_Construct)
	{
		uint64_t* tmp_Final_Input = NULL;
		std::vector<float> tmp_Charging_Mask;
		int tmp_Dimension = 0;
		int tmp_Input_Depth_Total = 0;
		int tmp_Tick = 0;
		int tmp_Con = 0;

		for (int cou_C = 0; cou_C < Constructs[p_Construct]->Connection_Count; cou_C++)
		{
			tmp_Con = Constructs[p_Construct]->Connections[cou_C];
			//---std::cout << "\n Construct[" << p_Construct << "] Connections[" << cou_C << "]: " << tmp_Con;
			//---std::cout << "\n Construct[" << tmp_Con << "] Output_Depth: " << Constructs[tmp_Con]->CAN->Output_Depth;
			//---std::cout << "\n Construct[" << tmp_Con << "] Output_Depth_2D: " << Constructs[tmp_Con]->CAN->Output_Depth_2D;
			//---std::cout << "\n Construct[" << tmp_Con << "] Output_Depth_3D: " << Constructs[tmp_Con]->CAN->Output_Depth_3D;
			tmp_Input_Depth_Total += Constructs[tmp_Con]->CAN->get_Output_Depth();
		}

		//---std::cout << "\n Input_Depth_Total: " << tmp_Input_Depth_Total;

		tmp_Final_Input = new uint64_t[tmp_Input_Depth_Total];
		tmp_Charging_Mask.resize(tmp_Input_Depth_Total);

		for (int cou_C = 0; cou_C < Constructs[p_Construct]->Connection_Count; cou_C++)
		{
			tmp_Con = Constructs[p_Construct]->Connections[cou_C];

			tmp_Dimension = Constructs[tmp_Con]->CAN->get_Dimension();

			//---std::cout << "\n Construct[" << tmp_Con << "] Dimension: " << tmp_Dimension;
			for (int cou_Index = 0; cou_Index < Constructs[tmp_Con]->CAN->get_Output_Depth(); cou_Index++)
			{
				if (tmp_Dimension == 1) { tmp_Final_Input[tmp_Tick] = Constructs[tmp_Con]->CAN->Output[cou_Index].Treetop->NID; tmp_Charging_Mask[tmp_Tick] = Constructs[tmp_Con]->CAN->Output[cou_Index].Charge; }
				if (tmp_Dimension == 2) { tmp_Final_Input[tmp_Tick] = Constructs[tmp_Con]->CAN->Output_2D[cou_Index].Treetop->NID; tmp_Charging_Mask[tmp_Tick] = Constructs[tmp_Con]->CAN->Output_2D[cou_Index].Charge; }
				if (tmp_Dimension == 3) { tmp_Final_Input[tmp_Tick] = Constructs[tmp_Con]->CAN->Output_3D[cou_Index].Treetop->NID; tmp_Charging_Mask[tmp_Tick] = Constructs[tmp_Con]->CAN->Output_3D[cou_Index].Charge; }

				tmp_Tick++;
			}
		}

		set_Input_uint(p_Construct, tmp_Input_Depth_Total, tmp_Final_Input);
		set_Input_Charging_Mask(p_Construct, tmp_Charging_Mask);

		delete[] tmp_Final_Input;
		tmp_Final_Input = NULL;
	}

	void pull_Chrono_From_Lower_Connection(const int p_Construct)
	{
		int tmp_Con = Constructs[p_Construct]->Connections[0]; //Get the construct we are pulling from.
		//---std::cout << "\n Pulling from: " << Constructs[Constructs[p_Construct]->Connections[0]]->Name;
		add_Chrono(p_Construct, get_Treetop_NID(tmp_Con));
	}

	//Note, only use this on 1D constructs.
	void pull_From_Lower_Connection(const int p_Construct, const int p_Lower_Connection)
	{
		uint64_t* tmp_Final_Input = NULL;
		int tmp_Dimension = 0;
		int tmp_Con = Constructs[p_Construct]->Connections[p_Lower_Connection]; //Get the construct we are pulling from.
		int tmp_Input_Depth_Total = Constructs[tmp_Con]->CAN->get_Output_Depth(); //Get the depth of the output of the construct to pull from.
		int tmp_Tick = 0;
		
		//---std::cout << "\n Input_Depth_Total: " << tmp_Input_Depth_Total;

		tmp_Final_Input = new uint64_t[tmp_Input_Depth_Total];

		tmp_Dimension = Constructs[tmp_Con]->CAN->get_Dimension();

		//---std::cout << "\n Construct[" << tmp_Con << "] Dimension: " << tmp_Dimension;
		for (int cou_Index = 0; cou_Index < tmp_Input_Depth_Total; cou_Index++)
		{
			if (tmp_Dimension == 1) { tmp_Final_Input[tmp_Tick] = Constructs[tmp_Con]->CAN->Output[cou_Index].Treetop->NID; }
			if (tmp_Dimension == 2) { tmp_Final_Input[tmp_Tick] = Constructs[tmp_Con]->CAN->Output_2D[cou_Index].Treetop->NID; }
			if (tmp_Dimension == 3) { tmp_Final_Input[tmp_Tick] = Constructs[tmp_Con]->CAN->Output_3D[cou_Index].Treetop->NID; }

			tmp_Tick++;
		}

		set_Input_uint(p_Construct, tmp_Input_Depth_Total, tmp_Final_Input);

		delete[] tmp_Final_Input;
		tmp_Final_Input = NULL;
	}

	c_Trace* get_Output_Trace(int p_Construct, int p_Index)
	{
		return Constructs[p_Construct]->CAN->get_Output(p_Index);
	}

	bool check_Output_Bounds(int p_Construct, int p_Output_Index)
	{
		if (!(Constructs[p_Construct]->CAN->get_Output_Depth() <= p_Output_Index)) 
		{
			std::cerr << "\n   --(O.o)~~   ERROR: check_Output_Bounds(p_Construct = " << Constructs[p_Construct]->Name << ", p_Output_Index = " << p_Output_Index << ")";

			return 0; 
		}
		return 1;
	}

	//Iterates through every output trace in the given index of the given upper tier construct.
	void pull_From_Upper_Index(int p_Construct_To, int p_Construct_From, int p_Index)
	{
		std::ofstream tmp_Output_File;
		
		c_Trace * tmp_Trace = NULL;

		c_Linked_List_Handler tmp_Pattern;

		c_Linked_List* tmp_LL_Pat = NULL;

		int tmp_Output_Depth = Constructs[p_Construct_From]->CAN->get_Output_Depth();
		int tmp_Dimension = Constructs[p_Construct_To]->CAN->get_Dimension();

		//std::cout << "\n Depth: " << tmp_Output_Depth;

		//Setup the output to hold the traces
		Constructs[p_Construct_To]->CAN->allocate_Output(tmp_Output_Depth, tmp_Dimension);

		//std::cout << "\n Dimension: " << tmp_Dimension;

		//---Constructs[p_Construct_To]->CAN->output_Output();

		for (int cou_Trace = 0; cou_Trace < tmp_Output_Depth; cou_Trace++)
		{
			tmp_Pattern.reset();

			tmp_Trace = get_Output_Trace(p_Construct_From, cou_Trace);

			//std::cout << "\n [" << cou_Trace << "] ";

			//std::cout << " { " << tmp_Trace->get_Pattern_Index(p_Index) << " }";

			//Constructs[p_Construct_To]->CAN->Output[cou_Trace].set_Depth(tmp_Pattern.Depth);

			Constructs[p_Construct_To]->CAN->backpropagate_NID_Into_Given_Index(tmp_Trace->get_Pattern_Index(p_Index), cou_Trace, tmp_Trace->Charge);
		}
		//---output_Output(p_Construct_To);

	}

	//This function moves a uint from a given construct input index to the output of a given construct by appending it. This allows you to take the backpropagated MSC, Chrono, etc pattern of treetops and append each treetop to the output file of the given construct. So you can then run 
	//Comment left here in case I come back to it. Delete if something has superceded it and this comment will not be potentially needed.

	//    ---==  set_input [Construct_ID] [INPUT_STRING]  ==---
		//Set the value to the passed 1D string of uint64_t
	void set_Input(int p_Construct, std::string p_Input)
	{
		Constructs[p_Construct]->CAN->set_Input_String(p_Input);
	}

	//    ---==  set_input_uint [Construct_ID] [ARRAY_DEPTH] [UINT_ARRAY]  ==---
		//Set the value to the passed 1D string of uint64_t
	void set_Input_uint(int p_Construct, int p_Depth, uint64_t* p_Input)
	{
		if (p_Construct == -1) { return; }
		Constructs[p_Construct]->CAN->set_Input(p_Input, p_Depth);
	}



	//    ---==========================================================================---
	//   ---============================================================================---
	//  ---==   Different ways of gathering nodes, individually, and the entire set.   ==---
	//   ---============================================================================---
	//    ---==========================================================================---

	//    ---==  gather_given_node [Construct_ID] [NID]  ==---
	//		Writes the given node's data down in the Constructs[p_Construct]->Output_File file.
	void gather_Given_Node(int p_Construct, uint64_t p_NID)
	{
		std::ofstream tmp_Output_File;
		c_Node* tmp_Node = NULL;

		tmp_Node = Nodes.get_Node_Ref_By_NID(p_NID);

		tmp_Output_File.open(Constructs[p_Construct]->Output_File, std::ios::app);

		write_Node_To_File(p_Construct, &tmp_Output_File, tmp_Node);

		tmp_Output_File.close();
	}

	//		Writes the given node's data down in the Constructs[p_Construct]->Output_File file.
	void gather_Given_Node_uint(int p_Construct, uint64_t p_NID)
	{
		std::ofstream tmp_Output_File;
		c_Node* tmp_Node = NULL;

		tmp_Node = Nodes.get_Node_Ref_By_NID(p_NID);

		tmp_Output_File.open(Constructs[p_Construct]->Output_File, std::ios::app);

		write_Node_To_File(p_Construct, &tmp_Output_File, tmp_Node, 1);

		tmp_Output_File.close();
	}

	//    ---==  gather_all_nodes [Construct_ID]  ==---
	//		This writes the entire network to the Constructs[p_Construct]->Output_File file. Note, the output patterns are treated as character.
		//It uses the passed Construct to output the nodes by putting it into the output of that Construct, then into the file.
	void gather_All_Nodes(int p_Construct)
	{
		std::ofstream tmp_Output_File;

		c_Node* tmp_Node;
		tmp_Node = Nodes.Root;

		tmp_Output_File.open(Constructs[p_Construct]->Output_File, std::ios::app);

		while (tmp_Node != NULL)
		{
			tmp_Output_File << "\n";

			write_Node_To_File(p_Construct, &tmp_Output_File, tmp_Node);

			tmp_Node = tmp_Node->Next;
		}

		tmp_Output_File.close();
	}

	//    ---==  gather_all_nodes_uint [Construct_ID]  ==---
	//		This writes the entire network to the Constructs[p_Construct]->Output_File file. Note, the output patterns are treated as uint.
	void gather_All_Nodes_uint(int p_Construct)
	{
		std::ofstream tmp_Output_File;

		c_Node* tmp_Node;
		tmp_Node = Nodes.Root;

		tmp_Output_File.open(Constructs[p_Construct]->Output_File, std::ios::app);

		while (tmp_Node != NULL)
		{
			tmp_Output_File << "\n";

			write_Node_To_File(p_Construct, &tmp_Output_File, tmp_Node, 1);

			tmp_Node = tmp_Node->Next;
		}

		tmp_Output_File.close();
	}

	void gather_Treetops(int p_Construct)
	{
		Constructs[p_Construct]->CAN->gather_Treetops();
	}

	//    ---==================================================================---
	//   ---====================================================================---
	//  ---==   The output of a given Construct is read into the output file.   ==---
	//   ---====================================================================---
	//    ---==================================================================---

		//==--- DIRECT_HOOK ---==//
	void output_Trace_To_File(std::ofstream* p_SF, c_Trace* p_Trace, int p_Output_Type)
	{
		std::cout << " NID: " << p_Trace->Treetop->NID;
		std::cout << " Charge: " << p_Trace->Charge;
		std::cout << " RC: " << p_Trace->RC;
		std::cout << " Depth: " << p_Trace->get_Output_Pattern_Size();

		*p_SF << "\n";

		*p_SF << p_Trace->Treetop->NID;
		*p_SF << " " << p_Trace->Charge;
		*p_SF << " " << p_Trace->RC;
		*p_SF << " " << p_Trace->get_Output_Pattern_Size();

		*p_SF << " ";
		std::string tmp_In = "";

		if (p_Trace->get_Output_Pattern_Size() > 0)
		{
			if (p_Output_Type == 0)
			{
				*p_SF << char(p_Trace->Pattern[0]);
				std::cout << "  " << char(p_Trace->Pattern[0]);
			}
			if (p_Output_Type == 1)
			{
				*p_SF << p_Trace->Pattern[0];
				std::cout << "  " << p_Trace->Pattern[0];
			}
		}

		for (int cou_Index = 1; cou_Index < p_Trace->get_Output_Pattern_Size(); cou_Index++)
		{
			if (p_Output_Type == 0)
			{
				*p_SF << char(p_Trace->Pattern[cou_Index]);
				std::cout << " " << char(p_Trace->Pattern[cou_Index]);
			}
			if (p_Output_Type == 1)
			{
				*p_SF << " " << p_Trace->Pattern[cou_Index];
				std::cout << " " << p_Trace->Pattern[cou_Index];
			}

		}
	}

		//==--- DIRECT_HOOK ---==//
	void output_3D_Trace_To_File(std::ofstream* p_SF, c_3D_Trace* p_Trace, int p_Output_Type)
	{
		std::cout << " NID: " << p_Trace->Treetop->NID;
		std::cout << " Charge: " << p_Trace->Charge;
		std::cout << " RC: " << p_Trace->RC;
		std::cout << " Depth_X: " << p_Trace->Depth_X;
		std::cout << " Depth_Y: " << p_Trace->Depth_Y;
		std::cout << " Depth_Z: " << p_Trace->Depth_Z;

		*p_SF << "\nNID ";

		*p_SF << p_Trace->Treetop->NID;
		*p_SF << " Charge " << p_Trace->Charge;
		*p_SF << " RC " << p_Trace->RC;
		*p_SF << " Depth_X " << p_Trace->Depth_X;
		*p_SF << " Depth_Y " << p_Trace->Depth_Y;
		*p_SF << " Depth_Z " << p_Trace->Depth_Z;

		*p_SF << " Pat ";
		std::string tmp_In = "";
		/*
		if (p_Trace->Depth > 0)
		{
			if (p_Output_Type == 0)
			{
				*p_SF << char(p_Trace->Pattern[0][0]);
				std::cout << "  " << char(p_Trace->Pattern[0][0]);
			}
			if (p_Output_Type == 1)
			{
				*p_SF << p_Trace->Pattern[0][0];
				std::cout << "  " << p_Trace->Pattern[0][0];
			}
		}
		*/
		*p_SF << "\n";
		std::cout << "\n";
		for (int cou_X = 0; cou_X < p_Trace->Depth_X; cou_X++)
		{
			for (int cou_Y = 0; cou_Y < p_Trace->Depth_Y; cou_Y++)
			{
				for (int cou_Z = 0; cou_Z < p_Trace->Depth_Z; cou_Z++)
				{
					if (p_Output_Type == 0)
					{
						*p_SF << char(p_Trace->Pattern[cou_X][cou_Y][cou_Z]);
						std::cout << char(p_Trace->Pattern[cou_X][cou_Y][cou_Z]) << " ";
					}
					if (p_Output_Type == 1)
					{
						*p_SF << " " << p_Trace->Pattern[cou_X][cou_Y][cou_Z];
						std::cout << " " << p_Trace->Pattern[cou_X][cou_Y][cou_Z];
					}
				}
				*p_SF << "\n";
				std::cout << "\n";
			}
			*p_SF << "\n";
			std::cout << "\n";
		}


	}

		//==--- DIRECT_HOOK ---==//
	void output_2D_Trace_To_File(std::ofstream* p_SF, c_2D_Trace* p_Trace, int p_Output_Type)
	{
		std::cout << " NID: " << p_Trace->Treetop->NID;
		std::cout << " Charge: " << p_Trace->Charge;
		std::cout << " RC: " << p_Trace->RC;
		std::cout << " Depth_X: " << p_Trace->Depth_X;
		std::cout << " Depth_Y: " << p_Trace->Depth_Y;

		*p_SF << "\nNID ";

		*p_SF << p_Trace->Treetop->NID;
		*p_SF << " Charge " << p_Trace->Charge;
		*p_SF << " RC " << p_Trace->RC;
		*p_SF << " Depth_X " << p_Trace->Depth_X;
		*p_SF << " Depth_Y " << p_Trace->Depth_Y;

		*p_SF << " Pat ";
		std::string tmp_In = "";
		/*
		if (p_Trace->Depth > 0)
		{
			if (p_Output_Type == 0)
			{
				*p_SF << char(p_Trace->Pattern[0][0]);
				std::cout << "  " << char(p_Trace->Pattern[0][0]);
			}
			if (p_Output_Type == 1)
			{
				*p_SF << p_Trace->Pattern[0][0];
				std::cout << "  " << p_Trace->Pattern[0][0];
			}
		}
		*/
		*p_SF << "\n";
		std::cout << "\n";
		for (int cou_X = 0; cou_X < p_Trace->Depth_X; cou_X++)
		{
			for (int cou_Y = 0; cou_Y < p_Trace->Depth_Y; cou_Y++)
			{
				if (p_Output_Type == 0)
				{
					*p_SF << char(p_Trace->Pattern[cou_X][cou_Y]);
					std::cout << char(p_Trace->Pattern[cou_X][cou_Y]) << " ";
				}
				if (p_Output_Type == 1)
				{
					*p_SF << " " << p_Trace->Pattern[cou_X][cou_Y];
					std::cout << " " << p_Trace->Pattern[cou_X][cou_Y];
				}
			}
			*p_SF << "\n";
			std::cout << "\n";
		}
	}
	

	//==--- DIRECT_HOOK ---==//
	//Character output for this one, the default, always string.
	//p_Output_Type: 0 = string, 1 = uint64_t
	void output_Output_To_File(int p_Construct, int p_Output_Type = 0)
	{
		std::ofstream tmp_Output_File;

		tmp_Output_File.open(Constructs[p_Construct]->Output_File, std::ios::app);

		// Check if the flag file exists and can be opened
		if (tmp_Output_File.is_open())
		{
			//tmp_Output_File << Constructs[p_Construct]->CAN->Output_Depth;

			//---std::cout << "\n Output_Depth: " << Constructs[p_Construct]->CAN->Output_Depth;
			//---std::cout << "\n Output_Depth_2D: " << Constructs[p_Construct]->CAN->Output_Depth_2D;
			//---std::cout << "\n Output_Depth_3D: " << Constructs[p_Construct]->CAN->Output_Depth_3D;

			//For every trace write the info to the file
			for (int cou_Trace = 0; cou_Trace < Constructs[p_Construct]->CAN->Output.size(); cou_Trace++)
			{
				//---std::cout << "\n Trace [" << cou_Trace << "]";

				output_Trace_To_File(&tmp_Output_File, &(Constructs[p_Construct]->CAN->Output[cou_Trace]), p_Output_Type);
			}
			for (int cou_Trace = 0; cou_Trace < Constructs[p_Construct]->CAN->Output_Depth_2D; cou_Trace++)
			{
				//---std::cout << "\n 2D_Trace [" << cou_Trace << "]";

				output_2D_Trace_To_File(&tmp_Output_File, &(Constructs[p_Construct]->CAN->Output_2D[cou_Trace]), p_Output_Type);
			}
			for (int cou_Trace = 0; cou_Trace < Constructs[p_Construct]->CAN->Output_Depth_3D; cou_Trace++)
			{
				//---std::cerr << "\n 3D_Trace [" << cou_Trace << "]";

				output_3D_Trace_To_File(&tmp_Output_File, &(Constructs[p_Construct]->CAN->Output_3D[cou_Trace]), p_Output_Type);
			}
		}
		tmp_Output_File.close();
	}

	//    ---==  gather_output [Construct_ID]  ==---
	//		Takes every trace in the given Constructs output trace array and writes them to the Constructs[p_Construct]->Output_File file, note the output state patterns are treated as char.
	void gather_Output(int p_Construct)
	{
		output_Output_To_File(p_Construct);
	}

	//    ---==  gather_output_uint [Construct_ID]  ==---
	//		Takes every trace in the given Constructs output trace array and writes them to the Constructs[p_Construct]->Output_File file, note the output state patterns are treated as uint.
	void gather_Output_uint(int p_Construct)
	{
		output_Output_To_File(p_Construct, 1);
	}


	//    ---=============================---
	//   ---===============================---
	//  ---==   Treetop node gathering.   ==---
	//   ---===============================---
	//    ---=============================---


		//==--- DIRECT_HOOK ---==//
	void write_Node_To_File(int p_Construct, std::ofstream* p_SF, c_Node* p_Node, int p_Output_Type = 0)
	{
		if ((p_SF == NULL) || (p_Node == NULL)) { return; }

		// Check if the flag file exists and can be opened
		if (p_SF->is_open())
		{
			*p_SF << p_Node->NID;
			*p_SF << " RC ";
			*p_SF << p_Node->RC;
			*p_SF << " #Denrites ";
			*p_SF << p_Node->Dendrite_Count;
			*p_SF << " Dendrites[] ";
			for (int cou_D = 0; cou_D < p_Node->Dendrite_Count; cou_D++)
			{
				*p_SF << "[" << cou_D << "] ";
				*p_SF << p_Node->Dendrites[cou_D]->NID;
				*p_SF << " Weight ";
				*p_SF << p_Node->Dendrite_Weights[cou_D] << " ";;
			}
			*p_SF << " #Axon_Hillocks ";
			*p_SF << p_Node->Axon_Hillock_Count;
			*p_SF << " ";
			for (int cou_H = 0; cou_H < p_Node->Axon_Hillock_Count; cou_H++)
			{
				*p_SF << " Hill " << cou_H << " ";
				*p_SF << " #Axons ";
				*p_SF << p_Node->Axon_Count[cou_H];
				*p_SF << " Axons[] ";
				for (int cou_A = 0; cou_A < p_Node->Axon_Count[cou_H]; cou_A++)
				{
					*p_SF << "[" << cou_A << "] ";
					*p_SF << p_Node->Axons[cou_H][cou_A]->NID;
				}
			}

			Constructs[p_Construct]->CAN->gather_Given_Trace(p_Node->NID);

			if (Constructs[p_Construct]->CAN->Output.size() > 0)
			{
				//The single node should only generate one trace which will be stored in the [0] index.
				*p_SF << " Symbol_Depth ";
				*p_SF << Constructs[p_Construct]->CAN->Output[0].get_Output_Pattern_Size();
				*p_SF << " Symbol ";
				for (int cou_Index = 0; cou_Index < Constructs[p_Construct]->CAN->Output[0].get_Output_Pattern_Size(); cou_Index++)
				{
					if (p_Output_Type == 0)
					{
						*p_SF << char(Constructs[p_Construct]->CAN->Output[0].get_Pattern_Index(cou_Index));
					}
					if (p_Output_Type == 1)
					{
						*p_SF << Constructs[p_Construct]->CAN->Output[0].get_Pattern_Index(cou_Index) << " ";
					}
				}
			}
		}
	}
	
		//==--- DIRECT_HOOK ---==//
	void write_Node_To_File_2D(int p_Construct, std::ofstream* p_SF, c_Node* p_Node, int p_Output_Type = 0)
	{
		if ((p_SF == NULL) || (p_Node == NULL)) { return; }

		// Check if the flag file exists and can be opened
		if (p_SF->is_open())
		{
			*p_SF << p_Node->NID;
			*p_SF << " RC ";
			*p_SF << p_Node->RC;
			*p_SF << " #Denrites ";
			*p_SF << p_Node->Dendrite_Count;
			*p_SF << " Dendrites[] ";
			for (int cou_D = 0; cou_D < p_Node->Dendrite_Count; cou_D++)
			{
				*p_SF << "[" << cou_D << "] ";
				*p_SF << p_Node->Dendrites[cou_D]->NID;
			}
			*p_SF << " #Axon_Hillocks ";
			*p_SF << p_Node->Axon_Hillock_Count;
			*p_SF << " ";
			for (int cou_H = 0; cou_H < p_Node->Axon_Hillock_Count; cou_H++)
			{
				*p_SF << " Hill " << cou_H << " ";
				*p_SF << " #Axons ";
				*p_SF << p_Node->Axon_Count[cou_H];
				*p_SF << " Axons[] ";
				for (int cou_A = 0; cou_A < p_Node->Axon_Count[cou_H]; cou_A++)
				{
					*p_SF << "[" << cou_A << "] ";
					*p_SF << p_Node->Axons[cou_H][cou_A]->NID;
				}
			}

			Constructs[p_Construct]->CAN->gather_Given_Trace(p_Node->NID);

			if (Constructs[p_Construct]->CAN->Output.size() > 0)
			{
				//The single node should only generate one trace which will be stored in the [0] index.
				*p_SF << " Symbol_Depth ";
				*p_SF << Constructs[p_Construct]->CAN->Output[0].get_Output_Pattern_Size();
				*p_SF << " Symbol ";
				for (int cou_Index = 0; cou_Index < Constructs[p_Construct]->CAN->Output[0].get_Output_Pattern_Size(); cou_Index++)
				{
					if (p_Output_Type == 0)
					{
						*p_SF << char(Constructs[p_Construct]->CAN->Output[0].get_Pattern_Index(cou_Index));
					}
					if (p_Output_Type == 1)
					{
						*p_SF << Constructs[p_Construct]->CAN->Output[0].get_Pattern_Index(cou_Index) << " ";
					}
				}
			}
		}
	}

	void save_Node_To_File(std::ofstream* p_SF, c_Node* p_Node, int p_Output_Type = 0)
	{
		if ((p_SF == NULL) || (p_Node == NULL)) { return; }

		// Check if the flag file exists and can be opened
		if (p_SF->is_open())
		{
			*p_SF << p_Node->NID;
			*p_SF << " ";
			*p_SF << p_Node->RC;
			*p_SF << " ";
			*p_SF << p_Node->Type;
			*p_SF << " ";
			*p_SF << p_Node->State;
			*p_SF << " ";
			*p_SF << p_Node->Dendrite_Count;
			*p_SF << " ";
			for (int cou_D = 0; cou_D < p_Node->Dendrite_Count; cou_D++)
			{
				*p_SF << p_Node->Dendrites[cou_D]->NID;
				*p_SF << " ";
			}
		}
	}

	void save_Constructs(std::ofstream* p_SF)
	{
		*p_SF << Construct_Count;

		for (int cou_Con=0;cou_Con<Construct_Count;cou_Con++)
		{
			*p_SF << "\n";
			*p_SF << Constructs[cou_Con]->Name << " ";
			*p_SF << Constructs[cou_Con]->CAN_Type << " ";
			*p_SF << Constructs[cou_Con]->CAN->State_Nodes_Index << " ";

			*p_SF << get_Base_Charge(cou_Con) << " ";
			*p_SF << get_Modifier_Charge(cou_Con) << " ";
			*p_SF << get_Action_Potential_Threshold(cou_Con) << " ";
			*p_SF << get_Charging_Tier(cou_Con) << " ";
		}
	}

	//Save the node network one node at a time.
	void save_Node_Network(std::ofstream* p_SF)
	{
		*p_SF << "\n";
		*p_SF << Nodes.Node_Count;

		c_Node* tmp_Node;
		tmp_Node = Nodes.Root;

		int tmp_Loading_Bar = int(float(Nodes.Node_Count) / 100.0);
		int tmp_NID = 0;
		std::cout << "\n Saving: " << Nodes.Node_Count << " nodes.";
		std::cout << "\n";
		for (int cou_Index = 0; cou_Index <= 100; cou_Index++)
		{
			std::cout << "_";
		}
		std::cout << "\n";
		while (tmp_Node != NULL)
		{
			if (!(tmp_NID % tmp_Loading_Bar)) { std::cout << "."; }
			tmp_NID++;

			*p_SF << "\n";

			save_Node_To_File(p_SF, tmp_Node);

			tmp_Node = tmp_Node->Next;
		}
	}

	void save_State_Trees(std::ofstream* p_SF)
	{
		*p_SF << "\n" << Nodes.State_Node_Tree_Count;

		for (int cou_State = 0; cou_State < Nodes.State_Node_Tree_Count; cou_State++)
		{
			Nodes.State_Nodes[cou_State]->save_Tree(p_SF);
		}
	}

	void save(std::string p_FName)
	{
		std::ofstream tmp_Output_File;

		tmp_Output_File.open(p_FName, std::ios::ate);

		if (tmp_Output_File.is_open())
		{
			save_Constructs(&tmp_Output_File);
			save_Node_Network(&tmp_Output_File);
			save_State_Trees(&tmp_Output_File);
		}

		tmp_Output_File.close();
	}

	void load_Constructs(std::ifstream* p_SF)
	{
		int tmp_Construct_Count;
		*p_SF >> tmp_Construct_Count;

		std::string tmp_Name = "";
		std::string tmp_Type = "";
		int tmp_State_Index = 0;

		float tmp_Base_Charge = 0.0;
		float tmp_Modifier_Charge = 0.0;
		float tmp_Action_Potential_Threshold = 0.0;
		int tmp_Charging_Tier = 0;

		for (int cou_Con = 0; cou_Con < tmp_Construct_Count; cou_Con++)
		{
			*p_SF >> tmp_Name;
			*p_SF >> tmp_Type;
			*p_SF >> tmp_State_Index;

			register_Construct(tmp_Type, tmp_Name);
			set_State_Nodes_Index(cou_Con, tmp_State_Index);

			*p_SF >> tmp_Base_Charge;
			*p_SF >> tmp_Modifier_Charge;
			*p_SF >> tmp_Action_Potential_Threshold;
			*p_SF >> tmp_Charging_Tier;

			set_Base_Charge(cou_Con, tmp_Base_Charge);
			set_Modifier_Charge(cou_Con, tmp_Modifier_Charge);
			set_Action_Potential_Threshold(cou_Con, tmp_Action_Potential_Threshold);
			set_Charging_Tier(cou_Con, tmp_Charging_Tier);
		}
	}

	void load_Node_Network(std::ifstream* p_SF)
	{
		uint64_t tmp_NID = 0;
		float tmp_RC = 0.0;
		int tmp_Type = 0;
		uint64_t tmp_State = 0;
		int tmp_Dendrite_Count = 0;
		int * tmp_Dendrite_NID = NULL;

		c_Node** tmp_Dendrites = NULL;
		c_Node* tmp_Node = NULL;

		uint64_t tmp_Node_Count = 0;
		
		*p_SF >> tmp_Node_Count;

		std::cout << "\n Found " << tmp_Node_Count << " nodes.";

		int tmp_Loading_Bar = int(float(tmp_Node_Count) / 100.0);
		std::cout << "\n";
		for (int cou_Index = 0; cou_Index < 100; cou_Index++)
		{
			std::cout << "_";
		}
		std::cout << "\n";
		for (int cou_Node = 0; cou_Node < tmp_Node_Count; cou_Node++)
		{
			if (!(cou_Node % tmp_Loading_Bar)) { std::cout << "."; }

			//---std::cout << "\n";
			*p_SF >> tmp_NID;
			//---std::cout << tmp_NID << " ";

			*p_SF >> tmp_RC;
			//---std::cout << tmp_RC << " ";
			*p_SF >> tmp_Type;
			//---std::cout << tmp_Type << " ";
			*p_SF >> tmp_State;
			//---std::cout << tmp_State << " ";

			//This is because node 0 is already made
			if (tmp_NID != 0)
			{
				tmp_Node = new_Node();
			}


			if (tmp_Node != NULL)
			{
				tmp_Node->set_Type(tmp_Type);
				tmp_Node->RC = tmp_RC;
			}

			*p_SF >> tmp_Dendrite_Count;
			//---std::cout << tmp_Dendrite_Count << " ";

			tmp_Dendrite_NID = new int[tmp_Dendrite_Count];
			tmp_Dendrites = new c_Node*[tmp_Dendrite_Count];

			for (int cou_D = 0; cou_D < tmp_Dendrite_Count; cou_D++)
			{
				*p_SF >> tmp_Dendrite_NID[cou_D];
				//---std::cout << tmp_Dendrite_NID[cou_D] << " ";

				tmp_Dendrites[cou_D] = Nodes.get_Node_Ref_By_NID(tmp_Dendrite_NID[cou_D]);
			}

			if (tmp_Dendrite_Count > 0) { create_Connections(tmp_Node, tmp_Dendrite_Count, tmp_Dendrites); }
		}

		if (tmp_Dendrite_NID != NULL) { delete[] tmp_Dendrite_NID; tmp_Dendrite_NID = NULL; }
		if (tmp_Dendrites != NULL) { delete[] tmp_Dendrites; tmp_Dendrites = NULL; }

		//Now gather state nodes.


	}

	void load_State_Trees(std::ifstream* p_SF)
	{
		//This number should exist already in this system from registering the constructs, but we gather it here today to say our...
		int tmp_State_Tree_Count = 0;

		*p_SF >> tmp_State_Tree_Count;
		
		std::cout << "\n Loading [ " << tmp_State_Tree_Count << " ] State Trees...";

		int tmp_State_Node_Count = 0;

		uint64_t tmp_NID = 0;
		uint64_t tmp_State = 0;

		for (int cou_ST = 0; cou_ST < tmp_State_Tree_Count; cou_ST++)
		{
			*p_SF >> tmp_State_Node_Count;

			std::cout << "\n Found [ " << tmp_State_Node_Count << " ] State Bindings...";

			for (int cou_SN = 0; cou_SN < tmp_State_Node_Count; cou_SN++)
			{
				*p_SF >> tmp_NID;
				*p_SF >> tmp_State;

				//---std::cout << "\n NID " << tmp_NID << " State " << tmp_State;
				
				Nodes.assign_State_Node(cou_ST, Nodes.get_Node_Ref_By_NID(tmp_NID), tmp_State);
			}
		}
	}

	void load(std::string p_FName)
	{
		std::ifstream tmp_Load_File;

		tmp_Load_File.open(p_FName);

		if (tmp_Load_File.is_open())
		{
			load_Constructs(&tmp_Load_File);
			load_Node_Network(&tmp_Load_File);
			load_State_Trees(&tmp_Load_File);
		}

		tmp_Load_File.close();
	}

	//    ---==  gather_treetop_node [Construct_ID]  ==---
	//		This writes the current treetop node of the given Construct to the Constructs[p_Construct]->Output_File file. This does not erase the file.
	void gather_Treetop_Node(int p_Construct)
	{
		std::ofstream tmp_Output_File;
		c_Node* tmp_Treetop = NULL;

		tmp_Treetop = get_Treetop(p_Construct);

		tmp_Output_File.open(Constructs[p_Construct]->Output_File, std::ios::app);

		write_Node_To_File(p_Construct, &tmp_Output_File, tmp_Treetop);

		tmp_Output_File.close();
	}

	//    ---==  gather_treetop_node_uint [Construct_ID]  ==---
	//		This writes the current treetop node of the given Construct to the Constructs[p_Construct]->Output_File file. This does not erase the file.
	void gather_Treetop_Node_uint(int p_Construct)
	{
		std::ofstream tmp_Output_File;
		c_Node* tmp_Treetop = NULL;

		tmp_Treetop = get_Treetop(p_Construct);

		tmp_Output_File.open(Constructs[p_Construct]->Output_File, std::ios::app);

		write_Node_To_File(p_Construct, &tmp_Output_File, tmp_Treetop, 1);

		tmp_Output_File.close();
	}

	//    ---==  gather_treetop_NID [Construct_ID]  ==---
	//		This writes only the NID of the current treetop to the file. Does not erase the file.
	void gather_Treetop_NID(int p_Construct)
	{
		std::ofstream tmp_Output_File;
		c_Node* tmp_Treetop = NULL;

		tmp_Treetop = get_Treetop(p_Construct);

		tmp_Output_File.open(Constructs[p_Construct]->Output_File, std::ios::app);

		if (tmp_Treetop != NULL)
		{
			tmp_Output_File << tmp_Treetop->NID;
		}
		else
		{
			tmp_Output_File << "NULL";
		}
		tmp_Output_File.close();
	}

	void write_Treetop_NID_To_Other_Input(int p_Construct_From, int p_Construct_To)
	{
		std::ofstream tmp_Output_File;
		c_Node* tmp_Treetop = NULL;

		tmp_Treetop = get_Treetop(p_Construct_From);

		tmp_Output_File.open(Constructs[p_Construct_To]->Input_File, std::ios::app);

		if (tmp_Treetop != NULL)
		{
			tmp_Output_File << tmp_Treetop->NID << " ";
		}
		else
		{
			tmp_Output_File << "0";
		}
		tmp_Output_File.close();
	}


	//    ---=============---
	//   ---===============---
	//  ---==   Config   ==---
	//   ---==============---
	//    ---============---
	
	int save_Config(int p_Construct)
	{
		std::ofstream file_Object(Constructs[p_Construct]->Config_File, std::ios::ate);

		// Check if the flag file exists and can be opened
		if (file_Object.is_open())
		{
			file_Object << "\nBase_Charge " << get_Base_Charge(p_Construct);
			file_Object << "\nModifier_Charge " << get_Modifier_Charge(p_Construct);
			file_Object << "\nAction_Potential_Threshold " << get_Action_Potential_Threshold(p_Construct);
			file_Object << "\nCharging_Tier " << get_Charging_Tier(p_Construct);
		}
		else
		{
			std::cerr << "\n \\(o.O)/ save_Config " << p_Construct << " could not open the file " << Constructs[p_Construct]->Config_File << " for writing!";
		}

		file_Object.close();
		return 1;
	}

	int update_Config(int p_Construct)
	{
		std::ifstream config_File(Constructs[p_Construct]->Config_File);

		std::string tmp_In = "";
		float tmp_float = 0.0;
		int tmp_Int = 0;

		// Check if the flag file exists and can be opened
		if (config_File.is_open())
		{
			while (!config_File.eof())
			{
				config_File >> tmp_In;

				if (tmp_In == "Base_Charge") { config_File >> tmp_float; set_Base_Charge(p_Construct, tmp_float); }// std::cout << "\nSetting Base_Charge to " << tmp_float; }
				if (tmp_In == "Modifier_Charge") { config_File >> tmp_float; set_Modifier_Charge(p_Construct, tmp_float); }//  std::cout << "\nSetting Modifier_Charge to " << tmp_float; }
				if (tmp_In == "Action_Potential_Threshold") { config_File >> tmp_float; set_Action_Potential_Threshold(p_Construct, tmp_float); }//  std::cout << "\nSetting Action_Potential_Threshold to " << tmp_float; }
				if (tmp_In == "Charging_Tier") { config_File >> tmp_Int; set_Charging_Tier(p_Construct, tmp_Int); }//  std::cout << "\nSetting Charging_Tier to " << tmp_Int; }
			}
		}
		else
		{
			std::cerr << "\n \\(o.O)/ update_Config " << p_Construct << " could not open the file " << Constructs[p_Construct]->Config_File << " for update!";
		}

		config_File.close();
		return 1;
	}

	void output_Config(int p_Construct)
	{
		Constructs[p_Construct]->CAN->output_Config();
	}

	void set_Input_Charging_Mask(const int p_Construct, std::vector<float> p_Input_Charging_Mask)
	{
		Constructs[p_Construct]->CAN->set_Input_Charging_Mask(p_Input_Charging_Mask);
	}

	//Hyperparams
	void set_Base_Charge(int p_Construct, float p_Base_Charge)
	{
		Constructs[p_Construct]->CAN->set_Base_Charge(p_Base_Charge);
	}

	void set_Modifier_Charge(int p_Construct, float p_Modifier_Charge)
	{
		Constructs[p_Construct]->CAN->set_Modifier_Charge(p_Modifier_Charge);
	}

	void set_Action_Potential_Threshold(int p_Construct, float p_Action_Potential_Threshold)
	{
		Constructs[p_Construct]->CAN->set_Action_Potential_Threshold(p_Action_Potential_Threshold);
	}

	void set_Charging_Tier(int p_Construct, int p_Charging_Tier)
	{
		Constructs[p_Construct]->CAN->set_Charging_Tier(p_Charging_Tier);
	}

	float get_Base_Charge(int p_Construct)
	{
		return Constructs[p_Construct]->CAN->get_Base_Charge();
	}

	float get_Modifier_Charge(int p_Construct)
	{
		return Constructs[p_Construct]->CAN->get_Modifier_Charge();
	}

	float get_Action_Potential_Threshold(int p_Construct)
	{
		return Constructs[p_Construct]->CAN->get_Action_Potential_Threshold();
	}

	int get_Charging_Tier(int p_Construct)
	{
		return Constructs[p_Construct]->CAN->get_Charging_Tier();
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
		for (int cou_Con = 0; cou_Con < Construct_Count; cou_Con++)
		{
			std::cout << "\n [" << cou_Con << "]: " << Constructs[cou_Con]->Name << " - " << Constructs[cou_Con]->CAN_Type;
		}
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
	int clear_Output(int p_Construct)
	{
		std::ofstream file_Object(Constructs[p_Construct]->Output_File, std::ios::ate);

		// Check if the flag file exists and can be opened
		if (!file_Object.is_open())
		{
			std::cerr << "\n clear_Output " << p_Construct << " could not open the file " << Constructs[p_Construct]->Output_File << " for clearing!";
		}

		file_Object.close();
		return 1;
	}


	//    ---==  output_newline  ==---
	int output_Newline(int p_Construct)
	{
		std::ofstream file_Object(Constructs[p_Construct]->Output_File, std::ios::app);

		// Check if the flag file exists and can be opened
		if (file_Object.is_open())
		{
			file_Object << "\n";
		}
		else
		{
			std::cerr << "\n clear_Output " << p_Construct << " could not open the file " << Constructs[p_Construct]->Output_File << " for newline!";
		}

		file_Object.close();
		return 1;
	}

	//    ---==  output_newline  ==---
	int write_Text(int p_Construct, std::string p_Text)
	{
		std::ofstream file_Object(Constructs[p_Construct]->Output_File, std::ios::app);

		// Check if the flag file exists and can be opened
		if (file_Object.is_open())
		{
			file_Object << p_Text;
		}
		else
		{
			std::cerr << "\n clear_Output " << p_Construct << " could not open the file " << Constructs[p_Construct]->Output_File << " for newline!";
		}

		file_Object.close();
		return 1;
	}

};

/** @}*/