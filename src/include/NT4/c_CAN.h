//The Current Active Node Scaffold for Trace Encoding (CAN) is the scaffold we use to iteratively encode traces.
//For example, a network with an I/O depth of 5 and a single tier w/ one 5 legged node may look like this:
// Memory_Tier[1]: [0] //The top node, represents one permutation of the input tier. Multiple Memory tiers.
// The memory tiers start at [1] since the State tier is technically Memory tier [0].
// State_Tier (0): [0] [1] [2] [3] [4] //This is the tier with the states. 
// Each [] represents a pointer to a node. We iterate from bottom to top, using get_State_Node,
// and get_Upper_Tier_Node() to fill it out tier by tier.

//--Not using a polymorphic base class here, different CANs may have wildly different features.
//--Once enough are written then we can go back and write one if need be.
//Scratch that, making a base class so that when an assembly is registered a CAN can (hue hue hue) be chosen and function as the interface.

//This is the base class for a CAN. It has the input, encode, and outputs.
class c_Base_CAN //Man
{
public:

	//The reference to the node network to query for nodes.
	c_Node_Network* NNet;

	//This is the index in the c_Node_Network::State_Nodes[] array to request from.
	int State_Nodes_Index;

	//The inputs for the CAN, they are uint64_t internally, the data is not modified so as long as the correct interface is used then none will be lost.
	//uint64_t* Input;
	//int Input_Depth;
	c_IO_Set Input;
	c_2D_IO_Set Input_2D;
	c_3D_IO_Set Input_3D;
	std::vector<double> Input_Charging_Mask;

	//The output is an array of traces retrieved from charging the buffers.
	std::vector<c_Trace> Output;
	c_2D_Trace* Output_2D;
	c_3D_Trace* Output_3D;
	int Output_Depth_2D;
	int Output_Depth_3D;

	//The buffer used during charging.
	c_Charging_Buffer tmp_Buffman;

	//This hyperparameter is used for the pyramidal constructs, or those with multiple layers. The network will start charging on this tier to filter out less associated dirty sub-symbols.
	int Charging_Tier;

	//These may vary wildly depending on the CAN structure, the scaffold will be completely internal.
	//Because of this we will declare the actual scaffold in the derived classes rather than here.
	//c_Node** Scaffold[2];
	//int State_Depth; //We track this so that if the input is changed we can still properly delete the scaffold.

	//==--   Member functions   --==//

	//This is called in the CAN constructor when you write it, this initializes all shared vars.
	void init()
	{
		NNet = NULL;

		Input.reset();
		Input_2D.reset();
		Input_3D.reset();

		Output_2D = NULL;
		Output_Depth_2D = 0;
		Output_3D = NULL;
		Output_Depth_3D = 0;

		State_Nodes_Index = 0;

		//HYPERRRRRRRRRRRRRRRRRRRRRRRRRR
		Charging_Tier = 0;

		Output.clear();
	}

	//Wipe the input array.
	void reset_Input()
	{
		Input.reset();
		Input_2D.reset();
		Input_3D.reset();
	}

	void reset_Output()
	{
		Output.clear();

		if (Output_2D != NULL) { delete[] Output_2D; Output_2D = NULL; }
		Output_Depth_2D = 0;
		if (Output_3D != NULL) { delete[] Output_3D; Output_3D = NULL; }
		Output_Depth_3D = 0;

		tmp_Buffman.reset();
	}

	int get_Output_Depth()
	{
		if (get_Dimension() == 1)
		{
			return Output.size();
		}
		if (get_Dimension() == 2)
		{
			return Output_Depth_2D;
		}
		if (get_Dimension() == 3)
		{
			return Output_Depth_3D;
		}
		return 0;
	}

	c_Trace * get_Output(const int p_Output)
	{
		if (p_Output > Output.size()) { std::cerr << "\n\n   V(>.>)--  Error: trace * get_Output size " << Output.size() << " is less than requested output : " << p_Output; }
		return &(Output[p_Output]);
	}

	void allocate_Output(const int p_Output_Depth, const int p_Dimension)
	{
		if (p_Dimension == 1)
		{
			Output.clear();
			Output.resize(p_Output_Depth);
		}
		if (p_Dimension == 2)
		{
			if (Output_2D != NULL) { delete[] Output_2D; Output_2D = NULL; }

			Output_2D = new c_2D_Trace[p_Output_Depth];
			Output_Depth_2D = p_Output_Depth;
		}
		if (p_Dimension == 3)
		{
			if (Output_3D != NULL) { delete[] Output_3D; Output_3D = NULL; }

			Output_3D = new c_3D_Trace[p_Output_Depth];
			Output_Depth_3D = p_Output_Depth;
		}
	}

	//This encodes the p_Input data, if the nodes aren't found they are created, used for training.
	virtual void encode()=0;

	//This sets up the scaffold as encode does, but it doesn't create nodes if they aren't found, they remain NULL in the scaffold, this we call a NULLCAN
	//Used for querying the network, you input, fill the NULLCAN, charge the network, then gather the outputs.
	//Suggested for use before encoding (if using learning mode and not locked_to_initial_training_mode) otherwise it will also find the current trace as the perfect match.
	virtual void query(int p_Charging_Style = -1, int p_Leg = 0, int* p_Legs = NULL)=0;

	virtual void check_Symbol()=0;

	//This allows for passing unordered sets of nodes
	virtual void submit_Set(uint64_t* p_Input, int p_Depth)=0;

	//This returns the treetop node at a given index, for most structures this will be a single node, but for those like stiched-base networks with a treetop node count equal to the input node count then you can access them by index.
	virtual c_Node* get_Treetop(int p_Index = 0)=0;

	//This outputs the symbols each node represents. Currently only works with 1D output.
	//p_Type: 0 = char, 1 = uint64_t
	virtual void output_Scaffold_Symbols(int p_Type = 0)=0;
	
	//Calculates and returns the number of current treetops.
	int get_Output_Depth(int p_Index, int p_Dimension)
	{
		if (p_Dimension == 1) { return Output.size(); }
		if (p_Dimension == 2) { return Output_Depth_2D; }
		if (p_Dimension == 3) { return Output_Depth_3D; }
	}

	//Returns the dimension of the data.
	virtual int get_Dimension() = 0;
	
	//Gets a single trace from a given node. Puts it into the output.
	virtual void gather_Given_Trace(uint64_t p_NID)=0;
	
	virtual void gather_All_Traces()=0;

	//Gathers the treetops, used to be at the end of query but decoupled for complex searches.
	virtual void gather_Treetops()=0;

	virtual void backpropagate_NID_Into_Given_Index(uint64_t p_NID, int p_Index, double p_Charge) = 0;

	void set_Input_Charging_Mask(std::vector<double> p_Input_Charging_Mask)
	{
		Input_Charging_Mask = p_Input_Charging_Mask;
	}

	void chrono_Shift()
	{
		for (int cou_Index = 0; cou_Index < (Input.Depth - 1); cou_Index++)
		{
			Input.set_Value(Input.get_Value(cou_Index + 1), cou_Index);
		}
		Input.set_Value(0, (Input.Depth - 1));
	}

	void add_Chrono(uint64_t p_Chrono)
	{
		/*
		for (int cou_Index = (Input.Depth - 1); cou_Index > 0; cou_Index--)
		{
			Input.set_Value(Input.get_Value(cou_Index - 1), cou_Index);
		}*/
		for (int cou_Index = 0; cou_Index < (Input.Depth - 1); cou_Index++)
		{
			Input.set_Value(Input.get_Value(cou_Index + 1), cou_Index);
		}
		Input.set_Value(p_Chrono, (Input.Depth - 1));
	}

	//Hyperparams
	void set_Base_Charge(double p_Base_Charge)
	{
		tmp_Buffman.set_Base_Charge(p_Base_Charge);
	}

	void set_Modifier_Charge(double p_Modifier_Charge)
	{
		tmp_Buffman.set_Modifier_Charge(p_Modifier_Charge);
	}

	void set_Action_Potential_Threshold(double p_Action_Potential_Threshold)
	{
		tmp_Buffman.set_Action_Potential_Threshold(p_Action_Potential_Threshold);
		//std::cout << "\n Setting Action_Potential_Threshold to " << p_Action_Potential_Threshold;
		//std::cout << ", currently: " << tmp_Buffman.get_Action_Potential_Threshold();
	}

	void set_Charging_Tier(int p_Charging_Tier)
	{
		Charging_Tier = p_Charging_Tier;
	}

	double get_Base_Charge()
	{
		return tmp_Buffman.get_Base_Charge();
	}

	double get_Modifier_Charge()
	{
		return tmp_Buffman.get_Modifier_Charge();
	}

	double get_Action_Potential_Threshold()
	{
		return tmp_Buffman.get_Action_Potential_Threshold();
	}

	double get_Charging_Tier()
	{
		return Charging_Tier;
	}

	void output_Config()
	{
		std::cout << "\n Config:";
		std::cout << "\n - Charging_Tier: " << Charging_Tier;
		std::cout << "\n - Base_Charge: " << get_Base_Charge();
		std::cout << "\n - Modifier_Charge: " << get_Modifier_Charge();
		std::cout << "\n - Action_Potential_Threshold: " << get_Action_Potential_Threshold();
	}

	//Associate the CAN with a network from which to draw nodes.
	void set_NNet(c_Node_Network* p_NNet)
	{
		NNet = p_NNet;
	}

	//Sets the index for the state_Node_Tree in the c_Node_Network::State_Nodes[]
	void set_State_Nodes_Index(int p_Index)
	{
		std::cout << "\n State_Nodes_Index set to " << p_Index;
		State_Nodes_Index = p_Index;
	}

	//This violates every good practice I know hue hue hue
	union tmp_Union
	{
		char C;
		uint64_t U;
	};

	//Sets the input to the given uint64_t array.
	//The input array is 1D, but for more complex constructs dimensional index slicing is used.
	void set_Input(uint64_t* p_Input, int p_Input_Depth)
	{
		Input.set_Array_Value(p_Input, p_Input_Depth);
	}

	//This is used for setting the input array to reflect a sequence of characters.
	void set_Input_String(std::string p_Input)
	{
		tmp_Union * tmp_Input = NULL;
		uint64_t * tmp_Input_U = NULL;

		tmp_Input = new tmp_Union[p_Input.length()];
		tmp_Input_U = new uint64_t[p_Input.length()];

		for (int cou_Index = 0; cou_Index < p_Input.length(); cou_Index++ )
		{
			tmp_Input[cou_Index].U = 0;
			tmp_Input_U[cou_Index] = 0;
			tmp_Input[cou_Index].C = p_Input[cou_Index];
			tmp_Input_U[cou_Index] = tmp_Input[cou_Index].U;
		}

		Input.set_Array_Value(tmp_Input_U, int(p_Input.length()));

		if (tmp_Input != NULL) { delete[] tmp_Input; }
		tmp_Input = NULL;
		if (tmp_Input_U != NULL) { delete[] tmp_Input_U; }
		tmp_Input_U = NULL;
	}

	//Sets the input to the given uint64_t array.
	//The input array is 1D, but for more complex constructs dimensional index slicing is used.
	void set_2D_Input(uint64_t** p_Input, int p_X_Depth, int p_Y_Depth)
	{
		Input_2D.set_Array_Value(p_Input, p_X_Depth, p_Y_Depth);
	}

	//This is used for setting the input array to reflect a 2d set of characters.
	void set_2D_Input_String(char ** p_Input, int p_X_Depth, int p_Y_Depth)
	{
		tmp_Union** tmp_Input = NULL;
		uint64_t** tmp_Input_U = NULL;

		tmp_Input = new tmp_Union*[p_X_Depth];
		tmp_Input_U = new uint64_t*[p_X_Depth];

		for (int cou_X = 0; cou_X < p_X_Depth; cou_X++)
		{
			tmp_Input[cou_X] = new tmp_Union[p_Y_Depth];
			tmp_Input_U[cou_X] = new uint64_t[p_Y_Depth];

			for (int cou_Y = 0; cou_Y < p_Y_Depth; cou_Y++)
			{
				tmp_Input[cou_X][cou_Y].U = 0;
				tmp_Input_U[cou_X][cou_Y] = 0;
				tmp_Input[cou_X][cou_Y].C = p_Input[cou_X][cou_Y];
				tmp_Input_U[cou_X][cou_Y] = tmp_Input[cou_X][cou_Y].U;
			}

		}

		Input_2D.set_Array_Value(tmp_Input_U, p_X_Depth, p_Y_Depth);

		for (int cou_X = 0; cou_X < p_X_Depth; cou_X++)
		{
			if (tmp_Input[cou_X] != NULL) { delete[] tmp_Input[cou_X]; }
			tmp_Input[cou_X] = NULL;
			if (tmp_Input_U[cou_X] != NULL) { delete[] tmp_Input_U[cou_X]; }
			tmp_Input_U[cou_X] = NULL;
		}
		if (tmp_Input != NULL) { delete[] tmp_Input; }
		tmp_Input = NULL;
		if (tmp_Input_U != NULL) { delete[] tmp_Input_U; }
		tmp_Input_U = NULL;
	}

	//Sets the input to the given uint64_t array.
	//The input array is 1D, but for more complex constructs dimensional index slicing is used.
	void set_3D_Input(uint64_t*** p_Input, int p_X_Depth, int p_Y_Depth, int p_Z_Depth)
	{
		Input_3D.set_Array_Value(p_Input, p_X_Depth, p_Y_Depth, p_Z_Depth);
	}

	//This is used for setting the input array to reflect a 3d set of characters.
	void set_3D_Input_String(char *** p_Input, int p_X_Depth, int p_Y_Depth, int p_Z_Depth)
	{
		tmp_Union*** tmp_Input = NULL;
		uint64_t*** tmp_Input_U = NULL;

		tmp_Input = new tmp_Union**[p_X_Depth];
		tmp_Input_U = new uint64_t**[p_X_Depth];

		for (int cou_X = 0; cou_X < p_X_Depth; cou_X++)
		{
			tmp_Input[cou_X] = new tmp_Union*[p_Y_Depth];
			tmp_Input_U[cou_X] = new uint64_t*[p_Y_Depth];

			for (int cou_Y = 0; cou_Y < p_Y_Depth; cou_Y++)
			{
				tmp_Input[cou_X][cou_Y] = new tmp_Union[p_Z_Depth];
				tmp_Input_U[cou_X][cou_Y] = new uint64_t[p_Z_Depth];

				for (int cou_Z = 0; cou_Z < p_Z_Depth; cou_Z++)
				{
					tmp_Input[cou_X][cou_Y][cou_Z].U = 0;
					tmp_Input_U[cou_X][cou_Y][cou_Z] = 0;
					tmp_Input[cou_X][cou_Y][cou_Z].C = p_Input[cou_X][cou_Y][cou_Z];
					tmp_Input_U[cou_X][cou_Y][cou_Z] = tmp_Input[cou_X][cou_Y][cou_Z].U;
				}
			}
		}

		Input_3D.set_Array_Value(tmp_Input_U, p_X_Depth, p_Y_Depth, p_Z_Depth);

		for (int cou_X = 0; cou_X < p_X_Depth; cou_X++)
		{
			for (int cou_Y = 0; cou_Y < p_Y_Depth; cou_Y++)
			{
				if (tmp_Input[cou_X][cou_Y] != NULL) { delete[] tmp_Input[cou_X][cou_Y]; }
				tmp_Input[cou_X][cou_Y] = NULL;
				if (tmp_Input_U[cou_X][cou_Y] != NULL) { delete[] tmp_Input_U[cou_X][cou_Y]; }
				tmp_Input_U[cou_X][cou_Y] = NULL;
			}
			if (tmp_Input[cou_X] != NULL) { delete[] tmp_Input[cou_X]; }
			tmp_Input[cou_X] = NULL;
			if (tmp_Input_U[cou_X] != NULL) { delete[] tmp_Input_U[cou_X]; }
			tmp_Input_U[cou_X] = NULL;
		}
		if (tmp_Input != NULL) { delete[] tmp_Input; }
		tmp_Input = NULL;
		if (tmp_Input_U != NULL) { delete[] tmp_Input_U; }
		tmp_Input_U = NULL;
	}


	int get_Top(c_Linked_List_Handler* p_Pattern_N)
	{
		int tmp_Top_X = 0;

		//We can iterate through given we know how big the linked list is.
		c_Linked_List* tmp_LL_Pat_X = p_Pattern_N->Root;

		for (int cou_Index = 0; cou_Index < p_Pattern_N->Depth; cou_Index++)
		{
			if (tmp_LL_Pat_X->Quanta >= tmp_Top_X)
			{
				//+1 as we want the depth, not the furthest index.
				tmp_Top_X = int(tmp_LL_Pat_X->Quanta + 1);
			}
			tmp_LL_Pat_X = tmp_LL_Pat_X->Next;
		}

		return tmp_Top_X;
	}

	//==--   Output Functions   --==//

	//Outputs the scaffold as addresses.
	virtual void output_Scaffold()=0;

	//Outputs the input array.
	void output_Input(int p_Type = 0)
	{
		std::cout << "\n 1D:\n";
		for (int cou_Index = 0; cou_Index < Input.Depth; cou_Index++)
		{
			if (p_Type == 0) { std::cout << "\n [" << cou_Index << "] > " << char(Input.Data[cou_Index]) << " <"; }
			if (p_Type == 1) { std::cout << "\n [" << cou_Index << "] > " << Input.Data[cou_Index] << " <"; }
			if (Input_Charging_Mask.size() > cou_Index) { std::cout << " {" << Input_Charging_Mask[cou_Index] << "}"; }
		}
		std::cout << "\n 2D:\n";
		Input_2D.output();
		std::cout << "\n 3D:\n";
		Input_3D.output();
	}

	//Outputs the input array.
	//p_Type: 0 == char(), 1 == uint64_t (native), 2 == double
	void output_Output(int p_Type = 0)
	{
		std::cout << "\n 1D: " << Output.size() << " 2D: " << Output_Depth_2D << " 3D: " << Output_Depth_3D;
		for (int cou_Output = 0; cou_Output < Output.size(); cou_Output++)
		{
			std::cout << "\n  1D_Trace[" << cou_Output << "]: ";

			//std::cout << " Pat_Depth: " << Output[cou_Output].Depth;
			Output[cou_Output].output(p_Type);
			/*
			for (int cou_Index = 0; cou_Index < Output[cou_Output].Depth; cou_Index++)
			{
				//std::cout << "\n [" << cou_Index << "] ";
				std::cout << " " << Output[cou_Output].Pattern[cou_Index];
			}
			*/
		}
		for (int cou_Output = 0; cou_Output < Output_Depth_2D; cou_Output++)
		{
			std::cout << "\n  2D_Trace[" << cou_Output << "]: ";

			Output_2D[cou_Output].output(p_Type);
			/*
			for (int cou_Index = 0; cou_Index < Output[cou_Output].Depth; cou_Index++)
			{
				//std::cout << "\n [" << cou_Index << "] ";
				std::cout << " " << Output[cou_Output].Pattern[cou_Index];
			}
			*/
		}
		for (int cou_Output = 0; cou_Output < Output_Depth_3D; cou_Output++)
		{
			std::cout << "\n  3D_Trace[" << cou_Output << "]: ";

			Output_3D[cou_Output].output(p_Type);
			/*
			for (int cou_Index = 0; cou_Index < Output[cou_Output].Depth; cou_Index++)
			{
				//std::cout << "\n [" << cou_Index << "] ";
				std::cout << " " << Output[cou_Output].Pattern[cou_Index];
			}
			*/
		}
		//std::cout << "\n   --==   /Output_Traces   ==--";
	}

	virtual void output_Scaffold_Tops()=0;

	//Each address is typecast to a char to give a pseudo-unique look to each node. For monke brain.
	virtual void output_Scaffold_Char()=0;
};
