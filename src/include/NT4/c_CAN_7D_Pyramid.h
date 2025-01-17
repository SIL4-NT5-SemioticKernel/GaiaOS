
//This functions to create a trace where all legs are connected to one node.
//Load up the input, hit encode(), boom you single permutation of all legs bound to that node.
//Most useful when each index in the input is a separate state space from the other indexes, sensors == good, language == shared states across words !good
class c_CAN_3D_Pyramid : public c_Base_CAN
{
public:

	//Each CAN node is just a pointer to a node in the network.
	//Scaffold[Tier][X][Y]
	//Tier_Depth == where T = current tier: (Input_Depth - T)
	c_Node********* Scaffold;

	int State_Depth_X; //We track this so that if the input is changed we can still properly delete the scaffold.
	int State_Depth_Y; //We track this so that if the input is changed we can still properly delete the scaffold.
	int State_Depth_Z; //We track this so that if the input is changed we can still properly delete the scaffold.
	int State_Depth_W; //We track this so that if the input is changed we can still properly delete the scaffold.
	int State_Depth_A; //We track this so that if the input is changed we can still properly delete the scaffold.
	int State_Depth_B; //We track this so that if the input is changed we can still properly delete the scaffold.
	int State_Depth_C; //We track this so that if the input is changed we can still properly delete the scaffold.
	int Top_Tier; //Whichever dimension is lowest is the limiting factor on the height for nodes with the same leg count. For every dimsension that 'closes' you want to drop a lower dimensional construct on top.
	//The temporary buffer used during charging.
	c_Charging_Buffer tmp_Buffman;

	c_CAN_3D_Pyramid()
	{
		NNet = NULL;

		Input.reset();
		Output = NULL;
		Output_Depth = 0;

		Scaffold = NULL;

		State_Depth_X = 0;
		State_Depth_Y = 0;
		State_Depth_Z = 0;
		State_Depth_W = 0;
		State_Depth_A = 0;
		State_Depth_B = 0;
		State_Depth_C = 0;
		State_Nodes_Index = 0;
	}

	~c_CAN_3D_Pyramid()
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
							for (int cou_Z = 0; cou_Z < (State_Depth_Z - cou_T); cou_Z++)
							{
							for (int cou_W = 0; cou_W < (State_Depth_W - cou_T); cou_W++)
							{
							for (int cou_A = 0; cou_A < (State_Depth_A - cou_T); cou_A++)
							{
							for (int cou_B = 0; cou_B < (State_Depth_B - cou_T); cou_B++)
							{
							for (int cou_C = 0; cou_C < (State_Depth_C - cou_T); cou_C++)
							{
							for (int cou_Z = 0; cou_Z < (State_Depth_Z - cou_T); cou_Z++)
							{
							for (int cou_Z = 0; cou_Z < (State_Depth_Z - cou_T); cou_Z++)
							{
								//Make sure to NULL the scaffold first.
								//DO NOT DELETE THEM, the addresses are for the node network, deleting them here will cause null pointer deletion in Node_Network.
								Scaffold[cou_T][cou_X][cou_Y][cou_Z] = NULL;
							}
							delete[] Scaffold[cou_T][cou_X][cou_Y];
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
		State_Depth_X = Input_3D.Depth[0];
		State_Depth_Y = Input_3D.Depth[1];
		State_Depth_Z = Input_3D.Depth[2];

		std::cout << "\n State_Depth_X: " << State_Depth_X;
		std::cout << "\n State_Depth_Y: " << State_Depth_Y;
		std::cout << "\n State_Depth_Z: " << State_Depth_Y;

		//Find the shortest side to set the top tier to as that is when the symbol will reduce in dimension.
		Top_Tier = State_Depth_X;
		if (State_Depth_Y < Top_Tier) { Top_Tier = State_Depth_Y; }
		if (State_Depth_Z < Top_Tier) { Top_Tier = State_Depth_Z; }

		//Define the tiers, the number of tiers to hold a pyramid is equal to the depth of the pattern it represents.
		Scaffold = new c_Node **** [Top_Tier];

		//Through the tiers we brings beers.
		for (int cou_T = 0; cou_T < Top_Tier; cou_T++)
		{
			//Through the steps Randolph Carter crept.
			Scaffold[cou_T] = new c_Node *** [State_Depth_X - cou_T];

			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				Scaffold[cou_T][cou_X] = new c_Node ** [State_Depth_Y - cou_T];

				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					Scaffold[cou_T][cou_X][cou_Y] = new c_Node *[State_Depth_Z - cou_T];

					for (int cou_Z = 0; cou_Z < (State_Depth_Z - cou_T); cou_Z++)
					{
						Scaffold[cou_T][cou_X][cou_Y][cou_Z] = NULL;
					}
				}
			}
		}
	}

	//This fills the state tier by querying the node network.
	//p_How: "Encode" == Create the nodes if they aren't found, "Query" == Returns NULL if they aren't found, used for checking if something has been encoded without modifying the network.
	void fill_State(std::string p_How = "Encode")
	{
		std::cout << "\n Encoding with State_Nodes_Index: " << State_Nodes_Index;
		if (p_How == "Encode")
		{
			for (int cou_X = 0; cou_X < State_Depth_X; cou_X++)
			{
				for (int cou_Y = 0; cou_Y < State_Depth_Y; cou_Y++)
				{
					for (int cou_Z = 0; cou_Z < State_Depth_Z; cou_Z++)
					{
						//Request the state node form the node network using get_State_Node so one is created if not found.
						//We have to make sure we request the correct state tree.
						Scaffold[0][cou_X][cou_Y][cou_Z] = NNet->get_State_Node(State_Nodes_Index, Input_3D.get_Value(cou_X, cou_Y, cou_Z));
						Scaffold[0][cou_X][cou_Y][cou_Z]->RC++;

						//If the node is also a treetop then set it to state/treetop.
						if ((Scaffold[0][cou_X][cou_Y][cou_Z]->Type == 2))
						{
							Scaffold[0][cou_X][cou_Y][cou_Z]->Type = 3;
						}
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
					for (int cou_Z = 0; cou_Z < State_Depth_Z; cou_Z++)
					{
						//Request the state node form the node network using get_State_Node so one is created if not found.
						//We have to make sure we request the correct state tree.
						Scaffold[0][cou_X][cou_Y][cou_Z] = NNet->does_State_Node_Exist(State_Nodes_Index, Input_3D.get_Value(cou_X, cou_Y, cou_Z));
					}
				}
			}
		}
	}

	//Just one node at the tippy top.
	//p_How: "Encode" == Create the nodes if they aren't found, "Query" == Returns NULL if they aren't found, used for checking if something has been encoded without modifying the network.
	void fill_Scaffold(std::string p_How = "Encode")
	{
		if ((State_Depth_X == 0) || (State_Depth_Y == 0) || (State_Depth_Z == 0))
		{
			return;
		}

		//These hold the matrix we use to get the nodes to submit to the get upper tier node.
		//It takes 8 points to define a cube, each point is a sub-symbol, so we gather the 8 sub-symbols and abstract them.
		c_Node* tmp_Nodes[8];

		for (int cou_T = 1; cou_T < Top_Tier; cou_T++)
		{
			std::cerr << "\n T: " << cou_T;
			//The extra -1 is so we don't step to the last node and reach into the void.
			/*
			We need to take them in a 2x2x2 block my dude don't be rude or crude unless you've got a job to Derude.
			
			0[0, 0, 0] 1[1, 0, 0]
			2[0, 1, 0] 3[1, 1, 0]

			4[0, 0, 1] 5[1, 0, 1]
			6[0, 1, 1] 7[1, 1, 1]

			*/
			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					for (int cou_Z = 0; cou_Z < (State_Depth_Z - cou_T); cou_Z++)
					{
						std::cerr << " - " << cou_X << ", " << cou_Y << ", " << cou_Z;

						//Get the legs for the node, the 2x2
						tmp_Nodes[0] = Scaffold[cou_T - 1][cou_X][cou_Y][cou_Z];
						tmp_Nodes[1] = Scaffold[cou_T - 1][cou_X + 1][cou_Y][cou_Z];
						tmp_Nodes[2] = Scaffold[cou_T - 1][cou_X][cou_Y + 1][cou_Z];
						tmp_Nodes[3] = Scaffold[cou_T - 1][cou_X + 1][cou_Y + 1][cou_Z];

						tmp_Nodes[4] = Scaffold[cou_T - 1][cou_X][cou_Y][cou_Z + 1];
						tmp_Nodes[5] = Scaffold[cou_T - 1][cou_X + 1][cou_Y][cou_Z + 1];
						tmp_Nodes[6] = Scaffold[cou_T - 1][cou_X][cou_Y + 1][cou_Z + 1];
						tmp_Nodes[7] = Scaffold[cou_T - 1][cou_X + 1][cou_Y + 1][cou_Z + 1];

						if (p_How == "Encode")
						{
							//We request a node that links 4 nodes together.
							Scaffold[cou_T][cou_X][cou_Y][cou_Z] = NNet->get_Upper_Tier_Node(tmp_Nodes, 8, 1);
							Scaffold[cou_T][cou_X][cou_Y][cou_Z]->RC++;
						}
						if (p_How == "Query")
						{
							//We request a node that links the entire state tier together, but do not create them.
							Scaffold[cou_T][cou_X][cou_Y][cou_Z] = NNet->does_Upper_Tier_Connection_Exist(tmp_Nodes, 8);
						}
					}
				}
			}
		}

		for (int cou_X = 0; cou_X < (State_Depth_X - Top_Tier - 1); cou_X++)
		{
			for (int cou_Y = 0; cou_Y < (State_Depth_Y - Top_Tier - 1); cou_Y++)
			{
				for (int cou_Z = 0; cou_Z < (State_Depth_Z - Top_Tier - 1); cou_Z++)
				{
					Scaffold[Top_Tier - 1][cou_X][cou_Y][cou_Z]->Type = 2;
				}
			}
		}
	}

	//Encodes a single trace, forcibly.
	//Arguments no longer used, need to remove during refactoria.
	void encode(uint64_t* p_Input = NULL, int p_Depth = 0)
	{
		std::cout << "\n\n<<-- Begin Encoding 3D -->>\n\n";

		//Set up the scaffold for the nodes to reside in as we build the trace.
		setup_CAN_Scaffold();

		output_Scaffold();

		//Work across the state tier to fill it out by requesting state nodes from the NNet, if not found they are created.
		fill_State("Encode");

		std::cerr << "\n State Filled.";
		output_Scaffold();

		//Fills the scaffold out by requesting nodes from the NNet and creating them if they aren't found.
		fill_Scaffold("Encode");

		//To be removed later after testing.
		//output_Input();
		output_Scaffold();
		output_Scaffold_Char();

		std::cout << "\n\n-- End Encoding --\n\n";
	}

	//Style determines whether it charges with normal submission of raw, or if it does the specific leg charging for Chrono.
	//Assumes the CAN is setup.
	void charge_Buffers(int p_Style = -1, int p_Leg = 0, int * p_Legs = NULL)
	{

		tmp_Buffman.reset();

		tmp_Buffman.Input_Position = 0;

		tmp_Buffman.charge_Outputs();

		for (int cou_T = 0; cou_T < Top_Tier; cou_T++)
		{
			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					for (int cou_Z = 0; cou_Z < (State_Depth_Z - cou_T); cou_Z++)
					{
						if (Scaffold[cou_T][cou_X][cou_Y][cou_Z] != NULL)
						{
							//std::cout << "\n\n ~~++==++~~ Charging Node: CAN[cou_T][" << cou_Input << "]: " << Scaffold[cou_T][cou_Input] << " ~ " << Scaffold[cou_T][cou_Input]->NID;

							//tmp_Buffman.submit(Scaffold[0][cou_Input], (1.0));
							if (p_Style == -1)
							{
								tmp_Buffman.submit(Scaffold[cou_T][cou_X][cou_Y][cou_Z], (10.0));
							}
							if (p_Style == 1)
							{
								//This style not used in pyramidal.
								//tmp_Buffman.charge_Given_Leg(Scaffold[cou_T][cou_X][cou_Y], (10.0), cou_Input);
							}
							if (p_Style == 2)
							{
								//p_Leg specifies which leg to charge in this function, p_Legs[] being unused.
								tmp_Buffman.charge_Given_Leg(Scaffold[cou_T][cou_X][cou_Y][cou_Z], (10.0), p_Leg);
							}
							if (p_Style == 3)
							{
								//p_Leg is used here as the count of elements in p_Legs[].
								tmp_Buffman.charge_Given_Legs(Scaffold[cou_T][cou_X][cou_Y][cou_Z], p_Leg, p_Legs, (10.0));
							}
						}
					}
				}
			}
		}

		//std::cout << "\n\n++++++++++++++++++++++++++++++ Before ++++++++++++++++++++++++++++++ \n\n";

		//tmp_Buffman.output_All_Buffers();

		tmp_Buffman.gather();

		//std::cout << "\n\n++++++++++++++++++++++++++++++ After ++++++++++++++++++++++++++++++ \n\n";

		//tmp_Buffman.output_All_Buffers();

		while (tmp_Buffman.flg_Not_Done)
		{
			//std::cout << "\n\n flg_Not_Done\n";

			tmp_Buffman.charge_Outputs();

			tmp_Buffman.gather();

			//tmp_Buffman.output_All_Buffers();
		}

		//std::cout << "\n\n Completed:\n";

		//tmp_Buffman.output_All_Buffers();

		//std::cout << "\n\n Treetops:\n";
		//tmp_Buffman.output_Treetops();

		c_Charging_Linked_List * tmp_Current_LL = NULL;
		tmp_Current_LL = tmp_Buffman.Treetops.Root;

		/*
		while (tmp_Current_LL != NULL)
		{
			tmp_Current_LL->NID->bp_O();

			tmp_Current_LL = tmp_Current_LL->Next;
		}
		*/
	}

	void gather_Treetops()
	{
		//---std::cout << "\n\n Gathering Treetops...";

		c_Charging_Linked_List* tmp_Current_LL = NULL;
		tmp_Current_LL = tmp_Buffman.Treetops.Root;

		if (Output != NULL) { delete[] Output; Output = NULL; }

		Output = new c_Trace[tmp_Buffman.Treetops.Depth];
		Output_Depth = tmp_Buffman.Treetops.Depth;

		int tmp_Current_Index = 0;

		c_Linked_List_Handler tmp_Pattern;

		c_Linked_List* tmp_LL_Pat = NULL;

		while (tmp_Current_LL != NULL)
		{
			tmp_Pattern.reset();

			//Get the pattern into a linked list
			tmp_Current_LL->NID->bp_Trace_O(&tmp_Pattern);

			std::cout << "\n NID: " << tmp_Current_LL->NID->NID;
			//---std::cout << "\n tmp_Pattern.Depth: " << tmp_Pattern.Depth;
			//---tmp_Pattern.output();

			//---std::cout << "\n tmp_Pattern.Depth: " << tmp_Pattern.Depth;
			//Copy the pattern over
			Output[tmp_Current_Index].set_Depth(tmp_Pattern.Depth);

			//---std::cout << "\n Output[" << tmp_Current_Index << "].Depth: " << Output[tmp_Current_Index].Depth;

			//---std::cout << "\n tmp_Pattern.Depth: " << tmp_Pattern.Depth;

			tmp_LL_Pat = tmp_Pattern.Root;

			//We can iterate through given we know how big the linked list is.
			for (int cou_Index = 0; cou_Index < tmp_Pattern.Depth; cou_Index++)
			{
				Output[tmp_Current_Index].set_Pattern_Index(tmp_LL_Pat->Quanta, cou_Index);
				tmp_LL_Pat = tmp_LL_Pat->Next;
			}

			//---std::cout << "\n tmp_Current_LL->NID->Current_Charge: " << tmp_Current_LL->NID->Current_Charge;
			//---std::cout << "\n tmp_Current_LL->Charge: " << tmp_Current_LL->Charge;
			//---std::cout << "\n tmp_Current_LL->NID->RC: " << tmp_Current_LL->NID->RC;
			Output[tmp_Current_Index].set_Charge(tmp_Current_LL->Charge);
			//Output[tmp_Current_Index].set_Charge(tmp_Current_LL->NID->Current_Charge);
			Output[tmp_Current_Index].set_RC(tmp_Current_LL->NID->RC);
			Output[tmp_Current_Index].set_Treetop(tmp_Current_LL->NID);

			tmp_Current_LL = tmp_Current_LL->Next;
			
			//---Output[tmp_Current_Index].output(0);
			//---Output[tmp_Current_Index].output(1);

			tmp_Current_Index++;
		}
	}

	//Gets a single trace from a given node. Puts it into the output.
	void gather_All_Traces()
	{
		std::cout << "\n\n Gathering All Traces!!!";



		c_Node* tmp_Node = NULL;
		tmp_Node = NNet->Root;

		if (Output != NULL) { delete[] Output; Output = NULL; }

		Output = new c_Trace[NNet->Node_Count];
		Output_Depth = NNet->Node_Count;

		int tmp_Current_Index = 0;

		c_Linked_List_Handler tmp_Pattern;

		c_Linked_List* tmp_LL_Pat = NULL;

		while (tmp_Node != NULL)
		{
			std::cout << "\nNode (" << tmp_Node->NID << ") ";
			tmp_Node->bp_O();


			tmp_Pattern.reset();

			//Get the pattern into a linked list
			tmp_Node->bp_Trace_O(&tmp_Pattern);

			std::cout << "\n NID: " << tmp_Node->NID;
			std::cout << "\n tmp_Pattern.Depth: " << tmp_Pattern.Depth;
			tmp_Pattern.output();

			std::cout << "\n tmp_Pattern.Depth: " << tmp_Pattern.Depth;
			//Copy the pattern over
			Output[tmp_Current_Index].set_Depth(tmp_Pattern.Depth);

			std::cout << "\n Output[" << tmp_Current_Index << "].Depth: " << Output[tmp_Current_Index].Depth;

			std::cout << "\n tmp_Pattern.Depth: " << tmp_Pattern.Depth;

			tmp_LL_Pat = tmp_Pattern.Root;

			//We can iterate through given we know how big the linked list is.
			for (int cou_Index = 0; cou_Index < tmp_Pattern.Depth; cou_Index++)
			{
				Output[tmp_Current_Index].set_Pattern_Index(tmp_LL_Pat->Quanta, cou_Index);
				tmp_LL_Pat = tmp_LL_Pat->Next;
			}

			std::cout << "\n tmp_Current_LL->NID->Current_Charge: " << tmp_Node->Current_Charge;
			std::cout << "\n tmp_Current_LL->NID->RC: " << tmp_Node->RC;
			Output[tmp_Current_Index].set_Charge(tmp_Node->Current_Charge);
			//Output[tmp_Current_Index].set_Charge(tmp_Current_LL->NID->Current_Charge);
			Output[tmp_Current_Index].set_RC(tmp_Node->RC);
			Output[tmp_Current_Index].set_Treetop(tmp_Node);

			Output[tmp_Current_Index].output(0);
			Output[tmp_Current_Index].output(1);

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

		if (Output != NULL) { delete[] Output; Output = NULL; }

		Output = new c_Trace[1];
		Output_Depth = 1;

		int tmp_Current_Index = 0;

		c_Linked_List_Handler tmp_Pattern;

		c_Linked_List* tmp_LL_Pat = NULL;

		tmp_Pattern.reset();

		//Get the pattern into a linked list
		tmp_Node->bp_Trace_O(&tmp_Pattern);

		//---std::cout << "\n NID: " << tmp_Node->NID;
		//---std::cout << "\n tmp_Pattern.Depth: " << tmp_Pattern.Depth;
		//---tmp_Pattern.output();

		//Copy the pattern over
		Output[tmp_Current_Index].set_Depth(tmp_Pattern.Depth);

		//---std::cout << "\n Output[" << tmp_Current_Index << "].Depth: " << Output[tmp_Current_Index].Depth;

		//---std::cout << "\n tmp_Pattern.Depth: " << tmp_Pattern.Depth;

		tmp_LL_Pat = tmp_Pattern.Root;

		//We can iterate through given we know how big the linked list is.
		for (int cou_Index = 0; cou_Index < tmp_Pattern.Depth; cou_Index++)
		{
			Output[tmp_Current_Index].set_Pattern_Index(tmp_LL_Pat->Quanta, cou_Index);
			tmp_LL_Pat = tmp_LL_Pat->Next;
		}

		//---std::cout << "\n tmp_Node->Current_Charge: " << tmp_Node->Current_Charge;
		//---std::cout << "\n tmp_Node->RC: " << tmp_Node->RC;
		Output[tmp_Current_Index].set_Charge(tmp_Node->Current_Charge);
		//Output[tmp_Current_Index].set_Charge(tmp_Current_LL->NID->Current_Charge);
		Output[tmp_Current_Index].set_RC(tmp_Node->RC);
		Output[tmp_Current_Index].set_Treetop(tmp_Node);

		//---Output[tmp_Current_Index].output(0);
		//---Output[tmp_Current_Index].output(1);

		tmp_Current_Index++;
	}


	//Fills out the NULLCAN, does not forcibly encode, and then charges the network.
	//Charging style determines leg charging. May move it to be a setting.
	//     -1: Charge by giving the input node the base charge, normal node charging.
	//      1: Use leg specific charging to charge every node based on its position in the input array. Node[1] would charge axons on Axon[1][n].
	//      2: Charge every input on the given p_Leg, used mainly when inputing single values to charge. Allows you to input Node[x] as the only input to the Chrono and charge it using Axon[4][n], or any axon hillock you choose.
	//		 This allows you to search forward by setting a node early in the time series and searching forwards, or setting it late in the time series and searching backwards.
	void query(uint64_t* p_Input = NULL, int p_Depth = 0, int p_Charging_Style = -1, int p_Leg = 0, int * p_Legs = NULL)
	{
		//Firstly we gather the inputly
		if ((p_Input != NULL) && (p_Depth > 0)) { set_Input(p_Input, p_Depth); }

		//Set up the scaffold for the nodes to reside in as we build the trace.
		setup_CAN_Scaffold();

		//Work across the state tier to fill it out by requesting state nodes from the NNet, if not found they are created.
		fill_State("Query");

		//Fills the scaffold out by requesting nodes from the NNet and creating them if they aren't found.
		fill_Scaffold("Query");

		charge_Buffers(p_Charging_Style, p_Leg, p_Legs);

		gather_Treetops();


		//To be removed later after testing.
		output_Input();
		output_Scaffold();
		output_Scaffold_Char();
		output_Output();
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
				return Scaffold[Top_Tier - 1][0][0][0];
			}

			//This construct is 4d which means at the highest point of 8 legged nodes before it turns to 2D then the dimension which is longer than the pyramid is tall will have treetops in a 1D line, like if you took a pyramid of Gaza and click-dragged it sideways with tracers on. Same with this 3D object except describing dragging a 3D 0bject is not intuitive to monke brayne.
			if (State_Depth_X >= Top_Tier)
			{
				return Scaffold[Top_Tier - 1][p_Index][0][0];
			}
			//>= in case X == Y and we don't want fall-through on a perfect one.
			if (State_Depth_Y >= Top_Tier)
			{
				return Scaffold[Top_Tier - 1][0][p_Index][0];
			}
			//>= in case X == Y == Z and we don't want fall-through on a perfect one.
			if (State_Depth_Z >= Top_Tier)
			{
				return Scaffold[Top_Tier - 1][0][0][p_Index];
			}
		}
		return NULL;
	}

	//Outputs the scaffold.
	void output_Scaffold()
	{
		std::cout << "\n  --==   CAN_Scaffold   ==--";

		for (int cou_T = 0; cou_T < Top_Tier; cou_T++)
		{
			std::cout << "\n <- Tier[" << cou_T << "] ->";
			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				std::cout << "\n";
				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					std::cout << "\n";
					for (int cou_Z = 0; cou_Z < (State_Depth_Z - cou_T); cou_Z++)
					{
						std::cout << " [(" << cou_X << ", " << cou_Y << ", " << cou_Z << ") ";
						if (Scaffold[cou_T][cou_X][cou_Y][cou_Z] != NULL) { std::cout << Scaffold[cou_T][cou_X][cou_Y][cou_Z]->NID; } else { std::cout << "NULL"; }
						std::cout << "]";
					}
				}
			}
		}
	}

	//Outputs the scaffold as character representing the address.
	void output_Scaffold_Char()
	{
		std::cout << "\n(" << this << ")\n";
		for (int cou_T = 0; cou_T < Top_Tier; cou_T++)
		{
			std::cout << "\n";
			for (int cou_X = 0; cou_X < (State_Depth_X - cou_T); cou_X++)
			{
				std::cout << "\n";
				for (int cou_Y = 0; cou_Y < (State_Depth_Y - cou_T); cou_Y++)
				{
					std::cout << "\n[";
					for (int cou_Z = 0; cou_Z < (State_Depth_Z - cou_T); cou_Z++)
					{
						std::cout << char(Scaffold[cou_T][cou_X][cou_Y][cou_Z]);
					}
					std::cout << "]";
				}
			}
		}
	}
};
