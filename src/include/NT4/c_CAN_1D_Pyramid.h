
//This functions to create a trace where all legs are connected to one node.
//Load up the input, hit encode(), boom you single permutation of all legs bound to that node.
//Most useful when each index in the input is a separate state space from the other indexes, sensors == good, language == shared states across words !good
class c_CAN_1D_Pyramid : public c_Base_CAN
{
public:

	//Each CAN node is just a pointer to a node in the network.
	//Scaffold[Tier][Index]
	//Tier_Depth == where T = current tier: (Input_Depth - T)
	c_Node*** Scaffold;

	int State_Depth; //We track this so that if the input is changed we can still properly delete the scaffold.

	c_CAN_1D_Pyramid()
	{
		init();

		Scaffold = NULL;

		State_Depth = 0;
	}

	~c_CAN_1D_Pyramid()
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
			for (int cou_T = 0; cou_T < State_Depth; cou_T++)
			{
				if (Scaffold[cou_T] != NULL)
				{
					for (int cou_Index = 0; cou_Index < (State_Depth - cou_T); cou_Index++)
					{
						//Make sure to NULL the scaffold first.
						//DO NOT DELETE THEM, the addresses are for the node network, deleting them here will cause null pointer deletion in Node_Network.
						Scaffold[cou_T][cou_Index] = NULL;
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
		State_Depth = Input.Depth;

		//Define the tiers, the number of tiers to hold a pyramid is equal to the depth of the pattern it represents.
		Scaffold = new c_Node ** [State_Depth];

		//Through the tiers we brings beers.
		for (int cou_T = 0; cou_T < State_Depth; cou_T++)
		{
			//Through the steps Randolph Carter crept.
			Scaffold[cou_T] = new c_Node * [State_Depth - cou_T];

			for (int cou_Index = 0; cou_Index < (State_Depth - cou_T); cou_Index++)
			{
				Scaffold[cou_T][cou_Index] = NULL;
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
			for (int cou_Index = 0; cou_Index < State_Depth; cou_Index++)
			{
				//Request the state node form the node network using get_State_Node so one is created if not found.
				//We have to make sure we request the correct state tree.
				Scaffold[0][cou_Index] = NNet->get_State_Node(State_Nodes_Index, Input.Data[cou_Index]);
				Scaffold[0][cou_Index]->RC++;

				//If the node is also a treetop then set it to state/treetop.
				if ((Scaffold[0][cou_Index]->Type == 2))
				{
					Scaffold[0][cou_Index]->Type = 3;
				}
			}
		}
		if (p_How == "Query")
		{
			for (int cou_Index = 0; cou_Index < State_Depth; cou_Index++)
			{
				//Request the state node form the node network using get_State_Node so one is created if not found.
				//We have to make sure we request the correct state tree.
				Scaffold[0][cou_Index] = NNet->does_State_Node_Exist(State_Nodes_Index, Input.Data[cou_Index]);
			}
		}
	}

	//Just one node at the tippy top.
	//p_How: "Encode" == Create the nodes if they aren't found, "Query" == Returns NULL if they aren't found, used for checking if something has been encoded without modifying the network.
	void fill_Scaffold(std::string p_How = "Encode")
	{
		if (State_Depth == 0)
		{
			return;
		}
		//---std::cout << "\n State_Depth: " << State_Depth;
		for (int cou_T = 1; cou_T < State_Depth; cou_T++)
		{
			//---std::cerr << "\n T: " << cou_T;
			//The extra -1 is so we don't step to the last node and reach into the void.
			for (int cou_Index = 0; cou_Index < (State_Depth - cou_T); cou_Index++)
			{
				//---std::cerr << " - " << cou_Index;
				if (p_How == "Encode")
				{
					//We request a node that links 2 nodes together.
					Scaffold[cou_T][cou_Index] = NNet->get_Upper_Tier_Node(&(Scaffold[cou_T - 1][cou_Index]), 2, 1, cou_T);
					Scaffold[cou_T][cou_Index]->RC++;
					Scaffold[cou_T][cou_Index]->rectify_Double_Legged_Nodes(); //Only need to do this for tiers 1+ as tier 0 doesn't have dendrites in this CAN.
				}
				if (p_How == "Query")
				{
					//---std::cout << "\n Query: Scaffold[" << cou_T << "][" << cou_Index << "]";
					//We request a node that links the entire state tier together, but do not create them.
					//Scaffold[cou_T][cou_Index] = NNet->does_Upper_Tier_Connection_Exist(&(Scaffold[cou_T][cou_Index]), 2);
					Scaffold[cou_T][cou_Index] = NNet->does_Upper_Tier_Connection_Exist(&(Scaffold[cou_T - 1][cou_Index]), 2);
					//---std::cout << " " << Scaffold[cou_T - 1][cou_Index] << " " << Scaffold[cou_T - 1][cou_Index - 1] << " => " << Scaffold[cou_T][cou_Index];
				}
			}
		}
		if (p_How == "Encode")
		{
			Scaffold[State_Depth - 1][0]->Type = 2;
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
		//output_Scaffold();
		//output_Scaffold_Char();
		/*
		if (Scaffold[State_Depth - 1][0] != NULL)
		{
			std::cout << "\nTreetop: " << Scaffold[State_Depth - 1][0]->NID;
		}
		else
		{
			std::cout << "\nTreetop: NULL";
		}
		*/
		//---std::cout << "\n\n-- End Encoding --\n\n";
	}

	

	//Style determines whether it charges with normal submission of raw, or if it does the specific leg charging for Chrono.
	//Assumes the CAN is setup.
	void charge_Buffers(int p_Style = -1, int p_Leg = 0, int * p_Legs = NULL)
	{
		tmp_Buffman.reset();

		tmp_Buffman.Input_Position = 0;

		tmp_Buffman.charge_Outputs();

		for (int cou_T = Charging_Tier; cou_T < State_Depth; cou_T++)
		{
			//---std::cout << "\n  ~~~~~~~~~~~~~~~~~~~~~ Charging Tier [" << cou_T << "]";
			for (int cou_Input = 0; cou_Input < (State_Depth - cou_T); cou_Input++)
			{
				if (Scaffold[cou_T][cou_Input] != NULL)
				{
					//---std::cout << "\n             -Scaffold[" << cou_T << "][" << cou_Input << "]";

					if (p_Style == -1)
					{
						tmp_Buffman.submit(Scaffold[cou_T][cou_Input], (tmp_Buffman.get_Base_Charge()));
					}
					if (p_Style == 1)
					{
						tmp_Buffman.charge_Given_Leg(Scaffold[cou_T][cou_Input], (tmp_Buffman.get_Base_Charge()), cou_Input);
					}
					if (p_Style == 2)
					{
						//p_Leg specifies which leg to charge in this function, p_Legs[] being unused.
						tmp_Buffman.charge_Given_Leg(Scaffold[cou_T][cou_Input], (tmp_Buffman.get_Base_Charge()), p_Leg);
					}
					if (p_Style == 3)
					{
						//p_Leg is used here as the count of elements in p_Legs[].
						tmp_Buffman.charge_Given_Legs(Scaffold[cou_T][cou_Input], p_Leg, p_Legs, (tmp_Buffman.get_Base_Charge()));
					}
					if (p_Style == 4)
					{
						tmp_Buffman.submit(Scaffold[cou_T][cou_Input], (tmp_Buffman.get_Base_Charge()));
					}
				}
			}
		}

		//---std::cout << "\n   {{{{{{{ 0 }}}}}}}}";
		//---tmp_Buffman.output_All_Buffers();

		tmp_Buffman.gather();

		//---std::cout << "\n   {{{{{{{ 1 }}}}}}}}";
		//---tmp_Buffman.output_All_Buffers();

		while (tmp_Buffman.flg_Not_Done)
		{
			tmp_Buffman.charge_Outputs();

			tmp_Buffman.gather();

			//---std::cout << "\n   {{{{{{{ 2 }}}}}}}}";
			//---tmp_Buffman.output_All_Buffers();
		}

		//---std::cout << "\n   {{{{{{{ 3 }}}}}}}}";
		//---tmp_Buffman.output_All_Buffers();
	}

	void gather_Treetops()
	{
		//---std::cout << "\n\n Gathering Treetops...";
		double tmp_Charge = 0.0;
		double tmp_H_Charge = tmp_Buffman.get_Treetops_Highest_Charge();
		if (tmp_H_Charge == 0) { return; }

		c_Charging_Linked_List* tmp_Current_LL = NULL;
		tmp_Current_LL = tmp_Buffman.Treetops.Root;

		Output.clear();
		Output.resize(tmp_Buffman.Treetops.Depth);

		int tmp_Current_Index = 0;

		c_Linked_List_Handler tmp_Pattern;

		c_Linked_List* tmp_LL_Pat = NULL;

		while (tmp_Current_LL != NULL)
		{
			tmp_Pattern.reset();

			//Get the pattern into a linked list
			tmp_Current_LL->NID->bp_Trace_O(&tmp_Pattern);

			//---std::cout << "\n NID: " << tmp_Current_LL->NID->NID;
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

			tmp_Charge = (tmp_Current_LL->Charge / tmp_H_Charge) * get_Base_Charge();
			Output[tmp_Current_Index].set_Charge(tmp_Charge);
			//Output[tmp_Current_Index].set_Charge(tmp_Current_LL->NID->Current_Charge);
			Output[tmp_Current_Index].set_RC(tmp_Current_LL->NID->RC);
			Output[tmp_Current_Index].set_Treetop(tmp_Current_LL->NID);

			tmp_Current_LL = tmp_Current_LL->Next;
			
			//---Output[tmp_Current_Index].output(0);
			//---Output[tmp_Current_Index].output(1);

			tmp_Current_Index++;
		}

		tmp_Buffman.reset_Treetops();
	}

	void backpropagate_NID_Into_Given_Index(uint64_t p_NID, int p_Index, double p_Charge)
	{
		c_Linked_List_Handler tmp_Pattern;

		c_Linked_List* tmp_LL_Pat = NULL;

		tmp_Pattern.reset();

		c_Node* tmp_Node = NNet->get_Node_Ref_By_NID(p_NID);

		if (tmp_Node == NULL) { std::cerr << "\n\n   v(o.O)V   Error in backpropagage_NID_Into_Given_Index, Node " << p_NID << " not found!"; return; }

		//Get the pattern into a linked list
		tmp_Node->bp_Trace_O(&tmp_Pattern);

		//Copy the pattern over
		Output[p_Index].set_Depth(tmp_Pattern.Depth);

		tmp_LL_Pat = tmp_Pattern.Root;

		//We can iterate through given we know how big the linked list is.
		for (int cou_Index = 0; cou_Index < tmp_Pattern.Depth; cou_Index++)
		{
			Output[p_Index].set_Pattern_Index(tmp_LL_Pat->Quanta, cou_Index);
			tmp_LL_Pat = tmp_LL_Pat->Next;
		}

		Output[p_Index].set_Charge(p_Charge);

		Output[p_Index].set_RC(tmp_Node->RC);
		Output[p_Index].set_Treetop(tmp_Node);
	}

	//Gets a single trace from a given node. Puts it into the output.
	void gather_All_Traces()
	{
		std::cout << "\n\n Gathering All Traces!!!";



		c_Node* tmp_Node = NULL;
		tmp_Node = NNet->Root;

		Output.clear();

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

			std::cout << "\n Output[" << tmp_Current_Index << "].Depth: " << Output[tmp_Current_Index].get_Output_Pattern_Size();

			std::cout << "\n tmp_Pattern.Depth: " << tmp_Pattern.Depth;

			tmp_LL_Pat = tmp_Pattern.Root;

			//We can iterate through given we know how big the linked list is.
			for (int cou_Index = 0; cou_Index < tmp_Pattern.Depth; cou_Index++)
			{
				Output[tmp_Current_Index].set_Pattern_Index(tmp_LL_Pat->Quanta, cou_Index);
				tmp_LL_Pat = tmp_LL_Pat->Next;
			}

			//std::cout << "\n tmp_Current_LL->NID->Current_Charge: " << tmp_Node->Current_Charge;
			std::cout << "\n tmp_Current_LL->NID->RC: " << tmp_Node->RC;
			//Output[tmp_Current_Index].set_Charge(tmp_Node->Current_Charge);
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

		Output.clear();

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
		//Output[tmp_Current_Index].set_Charge(tmp_Node->Current_Charge);
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
		//---output_Input();
		//output_Scaffold();
		//output_Scaffold_Char();
		//---output_Output();
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
		if (State_Depth > 0)
		{
			return Scaffold[State_Depth - 1][0];
		}
		return NULL;
	}

	//Returns the dimension of the data.
	int get_Dimension()
	{
		return 1;
	}

	//Outputs the scaffold.
	void output_Scaffold()
	{
		for (int cou_T = 0; cou_T < State_Depth; cou_T++)
		{
			std::cout << "\nT<" << cou_T << ">";
			for (int cou_Index = 0; cou_Index < (State_Depth - cou_T); cou_Index++)
			{
				std::cout << " [";
				if (Scaffold[cou_T][cou_Index] != NULL) { std::cout << Scaffold[cou_T][cou_Index]->NID; } else { std::cout << "NULL"; }
				std::cout << "] ";
			}
		}
	}

	//Outputs the scaffold as character representing the address.
	void output_Scaffold_Char()
	{
		for (int cou_T = 0; cou_T < State_Depth; cou_T++)
		{
			std::cout << "\nT<" << cou_T << "> [";
			for (int cou_Index = 0; cou_Index < (State_Depth - cou_T); cou_Index++)
			{
				std::cout << char(Scaffold[cou_T][cou_Index]);
			}
			std::cout << "]";
		}
	}

	void output_Scaffold_Tops()
	{
		std::vector<uint64_t> tmp_NID[2];
		std::vector<int> tmp_Index;
		std::vector<int> tmp_Tier;

		std::vector<uint64_t> xmp_NID;
		std::vector<int> xmp_Index;
		std::vector<int> xmp_Tier;

		tmp_NID[0].resize(State_Depth);
		tmp_NID[1].resize(State_Depth);
		tmp_Index.resize(State_Depth);
		tmp_Tier.resize(State_Depth);



		for (int cou_T = (State_Depth - 1); cou_T >= 0; cou_T--)
		{
			std::cout << "\nT<" << cou_T << "> [";
			for (int cou_Index = 0; cou_Index < (State_Depth - cou_T); cou_Index++)
			{
				std::cout << char(Scaffold[cou_T][cou_Index]);

				if (tmp_NID[0][cou_Index] == 0)
				{
					if (Scaffold[cou_T][cou_Index] != NULL)
					{
						std::cout << "\n\n[" << cou_Index << "]";

						for (int cou_Fill = cou_Index; cou_Fill <= (cou_Index + cou_T); cou_Fill++)
						{
							tmp_NID[0][cou_Fill] = Scaffold[cou_T][cou_Index]->NID;
							//tmp_NID[1][cou_Fill] = 0;
							tmp_Index[cou_Fill] = cou_Index;
							tmp_Tier[cou_Fill] = cou_T;

							std::cout << " >NID: " << tmp_NID[0][cou_Index];
							std::cout << " >NID: " << tmp_NID[1][cou_Index];
							std::cout << " >Index: " << tmp_Index[cou_Index];
							std::cout << " >Tier: " << tmp_Tier[cou_Index];
							std::cout << " >[___]   ";
						}
						tmp_NID[1][cou_Index] = Scaffold[cou_T][cou_Index]->NID;

						std::cout << " NID: " << tmp_NID[0][cou_Index];
						std::cout << " NID: " << tmp_NID[1][cou_Index];
						std::cout << " Index: " << tmp_Index[cou_Index];
						std::cout << " Tier: " << tmp_Tier[cou_Index];
						std::cout << " [___]   ";

						xmp_NID.push_back(Scaffold[cou_T][cou_Index]->NID);
						xmp_Index.push_back(cou_Index);
						xmp_Tier.push_back(cou_T);
					}
				}
			}
			std::cout << "]";
		}

		for (int cou_Index = 0; cou_Index < State_Depth; cou_Index++)
		{
			std::cout << "\n[" << cou_Index << "]";
			std::cout << " NID: " << tmp_NID[0][cou_Index];
			std::cout << " NID: " << tmp_NID[1][cou_Index];
			std::cout << " Index: " << tmp_Index[cou_Index];
			std::cout << " Tier: " << tmp_Tier[cou_Index];
			std::cout << " [___]";
			Scaffold[tmp_Tier[cou_Index]][tmp_Index[cou_Index]]->bp_O(0);
			std::cout << "[___]  ";
		}
		std::cout << "\n\n";
		for (int cou_Index = 0; cou_Index < State_Depth; cou_Index++)
		{
			if (tmp_NID[1][cou_Index] != NULL)
			{
				Scaffold[tmp_Tier[cou_Index]][tmp_Index[cou_Index]]->bp_O(0);
			}
		}
		
		bool tmp_Done = false;
		uint64_t xtmp_NID = 0;
		int xtmp_Index = 0;
		int xtmp_Tier = 0;

		while (!tmp_Done)
		{
			tmp_Done = true;

			for (int cou_Index = 1; cou_Index < xmp_NID.size(); cou_Index++)
			{
				if (xmp_Index[cou_Index - 1] > xmp_Index[cou_Index])
				{
					xtmp_Tier = xmp_Tier[cou_Index - 1];
					xtmp_Index = xmp_Index[cou_Index - 1];

					xmp_Tier[cou_Index - 1] = xmp_Tier[cou_Index];
					xmp_Index[cou_Index - 1] = xmp_Index[cou_Index];

					xmp_Tier[cou_Index] = xtmp_Tier;
					xmp_Index[cou_Index] = xtmp_Index;

					tmp_Done = false;
				}
			}
		}

		std::cout << "\n\n";
		for (int cou_Index = 0; cou_Index < xmp_NID.size(); cou_Index++)
		{
			//std::cout << "\n [" << cou_Index << "] ___ " << xmp_Tier[cou_Index] << " <___> " << xmp_Index[cou_Index];
			Scaffold[xmp_Tier[cou_Index]][xmp_Index[cou_Index]]->bp_O(0);
		}
	}

	//Outputs the scaffold as character representing the address. Currently only 1D supports this.
	void output_Scaffold_Symbols(int p_Type = 0)
	{
		for (int cou_T = 0; cou_T < State_Depth; cou_T++)
		{
			std::cout << "\nT<" << cou_T << "> ";
			for (int cou_Index = 0; cou_Index < (State_Depth - cou_T); cou_Index++)
			{
				std::cout << "[";
				if (Scaffold[cou_T][cou_Index] != NULL)
				{
					Scaffold[cou_T][cou_Index]->bp_O(p_Type);
				}
				else
				{
					std::cout << "NULL";
				}
				std::cout << "]";
			}
		}
	}
};
