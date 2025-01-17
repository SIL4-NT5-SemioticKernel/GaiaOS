
//This functions to create a trace where all legs are connected to one node.
//Load up the input, hit encode(), boom you single permutation of all legs bound to that node.
//Most useful when each index in the input is a separate state space from the other indexes, sensors == good, language == shared states across words !good
class c_CAN_2D_Pyramid : public c_Base_CAN
{
public:

	//Each CAN node is just a pointer to a node in the network.
	//Scaffold[Tier][X][Y]
	//Tier_Depth == where T = current tier: (Input_Depth - T)
	c_Node**** Scaffold;

	int State_Depth_X; //We track this so that if the input is changed we can still properly delete the scaffold.
	int State_Depth_Y; //We track this so that if the input is changed we can still properly delete the scaffold.
	int Top_Tier; //Whichever dimension is lowest is the limiting factor on the height for nodes with the same leg count. For every dimsension that 'closes' you want to drop a lower dimensional construct on top.

	c_CAN_2D_Pyramid()
	{
		init();

		Scaffold = NULL;

		State_Depth_X = 0;
		State_Depth_Y = 0;

		Top_Tier = 0;
	}

	~c_CAN_2D_Pyramid()
	{
		NNet = NULL;
		reset_Scaffold();
		reset_Input();
	}

	//Resets the CAN to NULL, and deletes the state tier + treetop.
	void reset_Scaffold()
	{
		if (Scaffold != NULL)
		{
			//For every tier
			for (int cou_T = 0; cou_T < Top_Tier; cou_T++)
			{
				if (Scaffold[cou_T] != NULL)
				{
					for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
					{
						for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
						{
							//Make sure to NULL the scaffold first.
							//DO NOT DELETE THEM, the addresses are for the node network, deleting them here will cause null pointer deletion in Node_Network.
							Scaffold[cou_T][cou_X][cou_Y] = NULL;
						}
						delete[] Scaffold[cou_T][cou_X];
						Scaffold[cou_T][cou_X] = NULL;
					}

					delete[] Scaffold[cou_T];
					Scaffold[cou_T] = NULL;
				}
			}
			delete[] Scaffold;
			Scaffold = NULL;
		}
	}

	//This sets up the actual CAN scaffold to use.
	//It assumes the input array is filled out, the size is based upon that array.
	void setup_CAN_Scaffold()
	{
		//Take it out back and put it down.
		reset_Scaffold();

		//Sizing her up!
		State_Depth_X = Input_2D.Depth[0];
		State_Depth_Y = Input_2D.Depth[1];

		Top_Tier = State_Depth_X;
		if (State_Depth_Y < State_Depth_X) { Top_Tier = State_Depth_Y; }

		//Define the tiers, the number of tiers to hold a pyramid is equal to the depth of the pattern it represents.
		Scaffold = new c_Node *** [Top_Tier];

		//Through the tiers we brings beers.
		for (int cou_T = 0; cou_T < Top_Tier; cou_T++)
		{
			//Through the steps Randolph Carter crept.
			Scaffold[cou_T] = new c_Node ** [State_Depth_X - cou_T];

			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				Scaffold[cou_T][cou_X] = new c_Node * [State_Depth_Y - cou_T];

				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					Scaffold[cou_T][cou_X][cou_Y] = NULL;
				}
			}
		}
	}

	//This fills the state tier by querying the node network.
	//p_How: "Encode" == Create the nodes if they aren't found, "Query" == Returns NULL if they aren't found, used for checking if something has been encoded without modifying the network.
	void fill_State(std::string p_How = "Encode")
	{
		//---std::cout << "\n Encoding with State_Nodes_Index: " << State_Nodes_Index;
		if (p_How == "Encode")
		{
			for (int cou_X = 0; cou_X < State_Depth_X; cou_X++)
			{
				for (int cou_Y = 0; cou_Y < State_Depth_Y; cou_Y++)
				{
					//Request the state node form the node network using get_State_Node so one is created if not found.
					//We have to make sure we request the correct state tree.
					Scaffold[0][cou_X][cou_Y] = NNet->get_State_Node(State_Nodes_Index, Input_2D.get_Value(cou_X, cou_Y));
					Scaffold[0][cou_X][cou_Y]->RC++;

					//If the node is also a treetop then set it to state/treetop.
					if ((Scaffold[0][cou_X][cou_Y]->Type == 2))
					{
						Scaffold[0][cou_X][cou_Y]->Type = 3;
					}
				}
			}
		}
		if (p_How == "Query")
		{
			for (int cou_X = 0; cou_X < State_Depth_X; cou_X++)
			{
				for (int cou_Y = 0; cou_Y < State_Depth_Y; cou_Y++)
				{
					//Request the state node form the node network using get_State_Node so one is created if not found.
					//We have to make sure we request the correct state tree.
					Scaffold[0][cou_X][cou_Y] = NNet->does_State_Node_Exist(State_Nodes_Index, Input_2D.get_Value(cou_X, cou_Y));
				}
			}
		}
	}

	//Just one node at the tippy top.
	//p_How: "Encode" == Create the nodes if they aren't found, "Query" == Returns NULL if they aren't found, used for checking if something has been encoded without modifying the network.
	void fill_Scaffold(std::string p_How = "Encode")
	{
		if ((State_Depth_X == 0) || (State_Depth_Y == 0))
		{
			return;
		}

		//These hold the matrix we use to get the nodes to submit to the get upper tier node.
		c_Node* tmp_Nodes[4];

		for (int cou_T = 1; cou_T < Top_Tier; cou_T++)
		{
			//---std::cerr << "\n T: " << cou_T;
			//The extra -1 is so we don't step to the last node and reach into the void.
			/*
			We need to take them in a 2x2 grid.
			0[0, 0] 1[1, 0]
			2[0, 1] 3[1, 1]
			*/
			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					//---std::cerr << " - " << cou_X << ", " << cou_Y;

					//Get the legs for the node, the 2x2
					tmp_Nodes[0] = Scaffold[cou_T - 1][cou_X][cou_Y]; //1

					tmp_Nodes[1] = Scaffold[cou_T - 1][cou_X][cou_Y + 1]; //2

					tmp_Nodes[2] = Scaffold[cou_T - 1][cou_X + 1][cou_Y]; //3

					tmp_Nodes[3] = Scaffold[cou_T - 1][cou_X + 1][cou_Y + 1]; //4

					if (p_How == "Encode")
					{
						//We request a node that links 4 nodes together.
						Scaffold[cou_T][cou_X][cou_Y] = NNet->get_Upper_Tier_Node(tmp_Nodes, 4, 1);
						Scaffold[cou_T][cou_X][cou_Y]->RC++;
						Scaffold[cou_T][cou_X][cou_Y]->rectify_Double_Legged_Nodes(); //Only need to do this for tiers 1+ as tier 0 doesn't have dendrites in this CAN.
					}
					if (p_How == "Query")
					{
						//We request a node that links the entire state tier together, but do not create them.
						Scaffold[cou_T][cou_X][cou_Y] = NNet->does_Upper_Tier_Connection_Exist(tmp_Nodes, 4);
					}
				}
			}
		}

		if (p_How == "Encode")
		{
			for (int cou_X = 0; cou_X < (State_Depth_X - Top_Tier - 1); cou_X++)
			{
				for (int cou_Y = 0; cou_Y < (State_Depth_Y - Top_Tier - 1); cou_Y++)
				{
					Scaffold[Top_Tier - 1][cou_X][cou_Y]->Type = 2;
				}
			}
		}
	}

	void check_Symbol()
	{
		//Set up the scaffold for the nodes to reside in as we build the trace.
		setup_CAN_Scaffold();

		//Work across the state tier to fill it out by requesting state nodes from the NNet, if not found they are created.
		fill_State("Query");

		//Fills the scaffold out by requesting nodes from the NNet and creating them if they aren't found.
		fill_Scaffold("Query");
	}

	//Encodes a single trace, forcibly.
	void encode()
	{
		//---std::cout << "\n\n<<-- Begin Encoding -->>\n\n";

		//Set up the scaffold for the nodes to reside in as we build the trace.
		setup_CAN_Scaffold();

		//---output_Scaffold();

		//Work across the state tier to fill it out by requesting state nodes from the NNet, if not found they are created.
		fill_State("Encode");

		//---std::cerr << "\n State Filled.";
		//---output_Scaffold();

		//Fills the scaffold out by requesting nodes from the NNet and creating them if they aren't found.
		fill_Scaffold("Encode");

		//To be removed later after testing.
		//output_Input();
		//---output_Scaffold();
		//output_Scaffold_Char();
		for (int cou_X = 0; cou_X < (State_Depth_X - (Top_Tier - 1)); cou_X++)
		{
			for (int cou_Y = 0; cou_Y < (State_Depth_Y - (Top_Tier - 1)); cou_Y++)
			{
				if (Scaffold[Top_Tier - 1][cou_X][cou_Y] != NULL)
				{
					std::cout << "\nTreetop: " << Scaffold[Top_Tier - 1][cou_X][cou_Y]->NID;
				}
				else
				{
					std::cout << "\nTreetop: NULL";
				}
			}
		}
	}

	//Style determines whether it charges with normal submission of raw, or if it does the specific leg charging for Chrono.
	//Assumes the CAN is setup.
	void charge_Buffers(int p_Style = -1, int p_Leg = 0, int * p_Legs = NULL)
	{
		tmp_Buffman.reset();

		tmp_Buffman.Input_Position = 0;

		tmp_Buffman.charge_Outputs();

		for (int cou_T = Charging_Tier; cou_T < Top_Tier; cou_T++)
		{
			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					if (Scaffold[cou_T][cou_X][cou_Y] != NULL)
					{
						if (p_Style == -1)
						{
							tmp_Buffman.submit(Scaffold[cou_T][cou_X][cou_Y], (tmp_Buffman.get_Base_Charge()));
						}
						if (p_Style == 1)
						{
							//This style not used in pyramidal.
							//tmp_Buffman.charge_Given_Leg(Scaffold[cou_T][cou_X][cou_Y], (tmp_Buffman.get_Base_Charge()), cou_Input);
						}
						if (p_Style == 2)
						{
							//p_Leg specifies which leg to charge in this function, p_Legs[] being unused.
							tmp_Buffman.charge_Given_Leg(Scaffold[cou_T][cou_X][cou_Y], (tmp_Buffman.get_Base_Charge()), p_Leg);
						}
						if (p_Style == 3)
						{
							//p_Leg is used here as the count of elements in p_Legs[].
							tmp_Buffman.charge_Given_Legs(Scaffold[cou_T][cou_X][cou_Y], p_Leg, p_Legs, (tmp_Buffman.get_Base_Charge()));
						}
					}
				}
			}
		}

		tmp_Buffman.gather();

		while (tmp_Buffman.flg_Not_Done)
		{
			tmp_Buffman.charge_Outputs();

			tmp_Buffman.gather();
		}

		c_Charging_Linked_List * tmp_Current_LL = NULL;
		tmp_Current_LL = tmp_Buffman.Treetops.Root;
	}

	void gather_Treetops()
	{
		double tmp_Charge = 0.0;
		double tmp_H_Charge = tmp_Buffman.get_Treetops_Highest_Charge();
		if (tmp_H_Charge == 0) { return; }
		//---std::cout << "\n\n\n\n\n Gathering Treetops...";

		c_Charging_Linked_List* tmp_Current_LL = NULL;
		tmp_Current_LL = tmp_Buffman.Treetops.Root;

		if (Output_2D != NULL) { delete[] Output_2D; Output_2D = NULL; }

		Output_2D = new c_2D_Trace[tmp_Buffman.Treetops.Depth];
		Output_Depth_2D = tmp_Buffman.Treetops.Depth;

		int tmp_Current_Index = 0;

		c_Linked_List_Handler tmp_Pattern;
		c_Linked_List_Handler tmp_Pattern_X;
		c_Linked_List_Handler tmp_Pattern_Y;
		int tmp_Top_X = 0; //Loop through tmp_Pattern_X to find the highest.
		int tmp_Top_Y = 0; //Loop through tmp_Pattern_Y to find the highest.

		c_Linked_List* tmp_LL_Pat = NULL;
		c_Linked_List* tmp_LL_Pat_X = NULL;
		c_Linked_List* tmp_LL_Pat_Y = NULL;

		while (tmp_Current_LL != NULL)
		{
			tmp_Top_X = 0;
			tmp_Top_Y = 0;

			tmp_Pattern.reset();
			tmp_Pattern_X.reset();
			tmp_Pattern_Y.reset();

			//Get the pattern into a linked list
			tmp_Current_LL->NID->bp_2D_Trace_O(&tmp_Pattern, &tmp_Pattern_X, &tmp_Pattern_Y);

			tmp_Top_X = get_Top(&tmp_Pattern_X);
			tmp_Top_Y = get_Top(&tmp_Pattern_Y);

			//Copy the pattern over
			Output_2D[tmp_Current_Index].set_Depth(tmp_Top_X, tmp_Top_Y);

			tmp_LL_Pat = tmp_Pattern.Root;
			tmp_LL_Pat_X = tmp_Pattern_X.Root;
			tmp_LL_Pat_Y = tmp_Pattern_Y.Root;

			//We can iterate through given we know how big the linked list is.
			for (int cou_Index = 0; cou_Index < tmp_Pattern.Depth; cou_Index++)
			{
				Output_2D[tmp_Current_Index].set_Pattern_Index(tmp_LL_Pat->Quanta, int(tmp_LL_Pat_X->Quanta), int(tmp_LL_Pat_Y->Quanta));
				tmp_LL_Pat = tmp_LL_Pat->Next;
				tmp_LL_Pat_X = tmp_LL_Pat_X->Next;
				tmp_LL_Pat_Y = tmp_LL_Pat_Y->Next;
			}

			tmp_Charge = (tmp_Current_LL->Charge / tmp_H_Charge) * get_Base_Charge();
			Output_2D[tmp_Current_Index].set_Charge(tmp_Charge);
			Output_2D[tmp_Current_Index].set_RC(tmp_Current_LL->NID->RC);
			Output_2D[tmp_Current_Index].set_Treetop(tmp_Current_LL->NID);

			tmp_Current_LL = tmp_Current_LL->Next;

			tmp_Current_Index++;
		}

		tmp_Buffman.reset_Treetops();
	}

	void backpropagate_NID_Into_Given_Index(uint64_t p_NID, int p_Index, double p_Charge)
	{
		c_Linked_List_Handler tmp_Pattern;
		c_Linked_List_Handler tmp_Pattern_X;
		c_Linked_List_Handler tmp_Pattern_Y;
		int tmp_Top_X = 0; //Loop through tmp_Pattern_X to find the highest.
		int tmp_Top_Y = 0; //Loop through tmp_Pattern_Y to find the highest.

		c_Linked_List* tmp_LL_Pat = NULL;
		c_Linked_List* tmp_LL_Pat_X = NULL;
		c_Linked_List* tmp_LL_Pat_Y = NULL;

		tmp_Pattern.reset();
		tmp_Pattern_X.reset();
		tmp_Pattern_Y.reset();

		c_Node* tmp_Node = NNet->get_Node_Ref_By_NID(p_NID);

		if (tmp_Node == NULL) { std::cerr << "\n\n   v(o.O)V   Error in backpropagage_NID_Into_Given_Index, Node " << p_NID << " not found!"; }

		//Get the pattern into a linked list
		tmp_Node->bp_2D_Trace_O(&tmp_Pattern, &tmp_Pattern_X, &tmp_Pattern_Y);

		tmp_Top_X = get_Top(&tmp_Pattern_X);
		tmp_Top_Y = get_Top(&tmp_Pattern_Y);

		//Copy the pattern over
		Output_2D[p_Index].set_Depth(tmp_Top_X, tmp_Top_Y);

		tmp_LL_Pat = tmp_Pattern.Root;
		tmp_LL_Pat_X = tmp_Pattern_X.Root;
		tmp_LL_Pat_Y = tmp_Pattern_Y.Root;

		//We can iterate through given we know how big the linked list is.
		for (int cou_Index = 0; cou_Index < tmp_Pattern.Depth; cou_Index++)
		{
			Output_2D[p_Index].set_Pattern_Index(tmp_LL_Pat->Quanta, int(tmp_LL_Pat_X->Quanta), int(tmp_LL_Pat_Y->Quanta));
			tmp_LL_Pat = tmp_LL_Pat->Next;
			tmp_LL_Pat_X = tmp_LL_Pat_X->Next;
			tmp_LL_Pat_Y = tmp_LL_Pat_Y->Next;
		}

		Output_2D[p_Index].set_Charge(p_Charge);
		Output_2D[p_Index].set_RC(tmp_Node->RC);
		Output_2D[p_Index].set_Treetop(tmp_Node);

	}

	//Gets a single trace from a given node. Puts it into the output.
	void gather_All_Traces()
	{
		c_Node* tmp_Node = NULL;
		tmp_Node = NNet->Root;

		//if (Output != NULL) { delete[] Output; Output = NULL; Output_Depth = 0; }
		Output.clear();
		if (Output_2D != NULL) { delete[] Output_2D; Output_2D = NULL; Output_Depth_2D = 0; }

		Output_2D = new c_2D_Trace[NNet->Node_Count];
		Output_Depth_2D = int(NNet->Node_Count);

		int tmp_Current_Index = 0;

		c_Linked_List_Handler tmp_Pattern;
		c_Linked_List_Handler tmp_Pattern_X;
		c_Linked_List_Handler tmp_Pattern_Y;
		int tmp_Top_X = 0; //Loop through tmp_Pattern_X to find the highest.
		int tmp_Top_Y = 0; //Loop through tmp_Pattern_Y to find the highest.

		c_Linked_List* tmp_LL_Pat = NULL;
		c_Linked_List* tmp_LL_Pat_X = NULL;
		c_Linked_List* tmp_LL_Pat_Y = NULL;

		while (tmp_Node != NULL)
		{
			tmp_Top_X = 0;
			tmp_Top_Y = 0;

			//If the node isn't 2D don't try to force it or you will crash.
			//if (tmp_Node->Dendrite_Count != 4) { tmp_Node = tmp_Node->Next; continue; }

			tmp_Pattern.reset();
			tmp_Pattern_X.reset();
			tmp_Pattern_Y.reset();

			//Get the pattern into a linked list
			tmp_Node->bp_2D_Trace_O(&tmp_Pattern, &tmp_Pattern_X, &tmp_Pattern_Y);

			tmp_Top_X = get_Top(&tmp_Pattern_X);
			tmp_Top_Y = get_Top(&tmp_Pattern_Y);

			//Copy the pattern over
			Output_2D[tmp_Current_Index].set_Depth(tmp_Top_X, tmp_Top_Y);

			tmp_LL_Pat = tmp_Pattern.Root;
			tmp_LL_Pat_X = tmp_Pattern_X.Root;
			tmp_LL_Pat_Y = tmp_Pattern_Y.Root;

			//We can iterate through given we know how big the linked list is.
			for (int cou_Index = 0; cou_Index < tmp_Pattern.Depth; cou_Index++)
			{
				Output_2D[tmp_Current_Index].set_Pattern_Index(tmp_LL_Pat->Quanta, int(tmp_LL_Pat_X->Quanta), int(tmp_LL_Pat_Y->Quanta));
				tmp_LL_Pat = tmp_LL_Pat->Next;
				tmp_LL_Pat_X = tmp_LL_Pat_X->Next;
				tmp_LL_Pat_Y = tmp_LL_Pat_Y->Next;
			}

			//Output_2D[tmp_Current_Index].set_Charge(tmp_Node->Current_Charge);
			//Output[tmp_Current_Index].set_Charge(tmp_Current_LL->NID->Current_Charge);
			Output_2D[tmp_Current_Index].set_RC(tmp_Node->RC);
			Output_2D[tmp_Current_Index].set_Treetop(tmp_Node);

			tmp_Current_Index++;

			tmp_Node = tmp_Node->Next;
		}
	}


	//Gets a single trace from a given node. Puts it into the output.
	void gather_Given_Trace(uint64_t p_NID)
	{
		//---std::cout << "\n\n Gathering Given Trace " << p_NID << "...";

		c_Node* tmp_Node = NULL;
		tmp_Node = NNet->get_Node_Ref_By_NID(p_NID);

		if (Output_2D != NULL) { delete[] Output_2D; Output_2D = NULL; Output_Depth_2D = 0; }

		Output_2D = new c_2D_Trace[1];
		Output_Depth_2D = 1;

		int tmp_Current_Index = 0;

		c_Linked_List_Handler tmp_Pattern;
		c_Linked_List_Handler tmp_Pattern_X;
		c_Linked_List_Handler tmp_Pattern_Y;

		tmp_Pattern.reset();
		tmp_Pattern_X.reset();
		tmp_Pattern_Y.reset();

		int tmp_Top_X = 0; //Loop through tmp_Pattern_X to find the highest.
		int tmp_Top_Y = 0; //Loop through tmp_Pattern_Y to find the highest.
		int tmp_Top_Z = 0; //Loop through tmp_Pattern_Y to find the highest.

		c_Linked_List* tmp_LL_Pat = NULL;
		c_Linked_List* tmp_LL_Pat_X = NULL;
		c_Linked_List* tmp_LL_Pat_Y = NULL;

		//Get the pattern into a linked list
		tmp_Node->bp_2D_Trace_O(&tmp_Pattern, &tmp_Pattern_X, &tmp_Pattern_Y);

		tmp_LL_Pat_X = tmp_Pattern_X.Root;

		//We can iterate through given we know how big the linked list is.
		tmp_Top_X = get_Top(&tmp_Pattern_X);
		tmp_Top_Y = get_Top(&tmp_Pattern_Y);

		//Copy the pattern over
		Output_2D[0].set_Depth(tmp_Top_X, tmp_Top_Y);

		tmp_LL_Pat = tmp_Pattern.Root;
		tmp_LL_Pat_X = tmp_Pattern_X.Root;
		tmp_LL_Pat_Y = tmp_Pattern_Y.Root;

		//We can iterate through given we know how big the linked list is.
		for (int cou_Index = 0; cou_Index < tmp_Pattern.Depth; cou_Index++)
		{
			Output_2D[0].set_Pattern_Index(tmp_LL_Pat->Quanta, int(tmp_LL_Pat_X->Quanta), int(tmp_LL_Pat_Y->Quanta));

			tmp_LL_Pat = tmp_LL_Pat->Next;
			tmp_LL_Pat_X = tmp_LL_Pat_X->Next;
			tmp_LL_Pat_Y = tmp_LL_Pat_Y->Next;
		}

		//No charge set here
		Output_2D[0].set_RC(tmp_Node->RC);
		Output_2D[0].set_Treetop(tmp_Node);

	}


	//Fills out the NULLCAN, does not forcibly encode, and then charges the network.
	//Charging style determines leg charging. May move it to be a setting.
	//     -1: Charge by giving the input node the base charge, normal node charging.
	//      1: Use leg specific charging to charge every node based on its position in the input array. Node[1] would charge axons on Axon[1][n].
	//      2: Charge every input on the given p_Leg, used mainly when inputing single values to charge. Allows you to input Node[x] as the only input to the Chrono and charge it using Axon[4][n], or any axon hillock you choose.
	//		 This allows you to search forward by setting a node early in the time series and searching forwards, or setting it late in the time series and searching backwards.
	void query(int p_Charging_Style = -1, int p_Leg = 0, int * p_Legs = NULL)
	{
		//Set up the scaffold for the nodes to reside in as we build the trace.
		setup_CAN_Scaffold();

		//Work across the state tier to fill it out by requesting state nodes from the NNet, if not found they are created.
		fill_State("Query");

		//Fills the scaffold out by requesting nodes from the NNet and creating them if they aren't found.
		fill_Scaffold("Query");

		charge_Buffers(p_Charging_Style, p_Leg, p_Legs);

		//gather_Treetops();


		//To be removed later after testing.
		//output_Input();
		//output_Scaffold();
		//output_Scaffold_Char();
		//output_Output();
	}

	//This allows for passing unordered sets of nodes
	void submit_Set(uint64_t* p_Input, int p_Depth)
	{
		//Firstly we gather the inputly
		set_Input(p_Input, p_Depth);

		//We only do the query on the state tier. This is because the nodes will not be in a coherent relationship to each other, only their presence is important.
		//The node adress being read in as the state means that the unordered set can be read in to the state tier because the shared memory space precludes duplicates, you just ignore any higher tiers than 0.
		//Work across the state tier to fill it out by requesting state nodes from the NNet, if not found they are created.
		fill_State("Query");

		//Then charge the buffers as normal.
		charge_Buffers();
	}

	//Gets the current treetop at the given index.
	c_Node* get_Treetop(int p_Index = -1)
	{
		if (Top_Tier > 0)
		{
			if (p_Index == -1)
			{
				return Scaffold[Top_Tier - 1][0][0];
			}

			//This construct is 2d which means at the highest point of 4 legged nodes before it turns to 2D then the dimension which is longer than the pyramid is tall will have treetops in a 1D line, like if you took a pyramid of Gaza and click-dragged it sideways with tracers on.
			if (State_Depth_X >= Top_Tier)
			{
				return Scaffold[Top_Tier - 1][p_Index][0];
			}
			//>= in case X == Y and we don't want fall-through on a perfect one.
			if (State_Depth_Y >= Top_Tier)
			{
				return Scaffold[Top_Tier - 1][0][p_Index];
			}
		}
		return NULL;
	}

	//Returns the dimension of the data.
	int get_Dimension()
	{
		return 2;
	}

	//Outputs the scaffold.
	void output_Scaffold()
	{
		for (int cou_T = 0; cou_T < Top_Tier; cou_T++)
		{
			std::cout << "\n <- Tier[" << cou_T << "] ->\n";
			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				std::cout << "\n";
				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					std::cout << " [";
					if (Scaffold[cou_T][cou_X][cou_Y] != NULL) { std::cout << Scaffold[cou_T][cou_X][cou_Y]->NID; } else { std::cout << "NULL"; }
					std::cout << "] ";
				}
			}
			std::cout << "\n";
		}
	}

	//Outputs the scaffold as character representing the address.
	void output_Scaffold_Char()
	{
		for (int cou_T = 0; cou_T < Top_Tier; cou_T++)
		{
			std::cout << "\n";
			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					std::cout << "[";
					std::cout << char(Scaffold[cou_T][cou_X][cou_Y]);
					std::cout << "]";
				}
			}
		}
	}

	void output_Scaffold_Tops()
	{

	}

	//Outputs the scaffold as character representing the address. Currently only 1D supports this.
	void output_Scaffold_Symbols(int p_Type = 0)
	{
	}
};
