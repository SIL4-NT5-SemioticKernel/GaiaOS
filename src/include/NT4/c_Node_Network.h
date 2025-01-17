
class c_Node_Network
{
public:

	//This is what holds all the nodes. It allows us to search by NID.
	//A node is created then added here. 
	c_Fractal_Tree Nodes;

	//This is the root node of the linked list, the linked list being the node soup.
	c_Node* Root;

	//Tracks the current node in the linked list, the last one in line for creation and appending of new nodes.
	c_Node** Current_Node;

	//We use this to assign Node IDs (NIDs) and then to iteratively delete the allocated nodes by querying the tree and deleting them.
	//It's slow to do it that way but we won't be killing the network except at shutdown.
	uint64_t Node_Count;

	//The current widest row in the network on last evaluation.
	float Fat_Tier;

	//This holds the state trees for each construct.
	//Even if two constructs have the same state input you may want to keep them separate, so you need multiple state trees.
	//The state tree does not create new nodes, it only tracks them.
	c_Fractal_Tree** State_Nodes;
	int State_Node_Tree_Count;

	//On creation a default construct at index 0 is registered.
	c_Node_Network()
	{
		Node_Count = 0;
		State_Node_Tree_Count = 1;
		State_Nodes = new c_Fractal_Tree * [1];
		State_Nodes[0] = new c_Fractal_Tree;

		Root = NULL;
		Current_Node = &Root;

		//When loading nodes remember that this one is created in the constructor so it will need skipped during the loading/saving phase.
		new_Node();
		Root->set_Type(0);
		Root->bind_State(0);
		Fat_Tier = 0;
	}


	~c_Node_Network()
	{
		std::cout << "\n ~c_Node_Network " << (Node_Count) << " " << this << ".........."; std::cout.flush();
		//delete Root;

		c_Node* tmp_LL = Root;
		Root = NULL;
		c_Node* tmp_LL_Next = NULL;

		while (tmp_LL != NULL)
		{
			tmp_LL_Next = tmp_LL->Next;
			tmp_LL->Next = NULL;
			delete tmp_LL;
			tmp_LL = NULL;
			tmp_LL = tmp_LL_Next;
		}

		delete Root;
		Root = NULL;

		std::cout << "\n ~~~c_Node_Network " << Node_Count << " " << this << ".........."; std::cout.flush();
	}

	int register_New_Construct()
	{
		c_Fractal_Tree** tmp_State_Nodes;

		tmp_State_Nodes = new c_Fractal_Tree * [State_Node_Tree_Count];

		for (int cou_Index = 0; cou_Index < State_Node_Tree_Count; cou_Index++)
		{
			tmp_State_Nodes[cou_Index] = State_Nodes[cou_Index];
			State_Nodes[cou_Index] = NULL;
		}

		if (State_Nodes != NULL) { delete[] State_Nodes; }

		State_Node_Tree_Count++;

		State_Nodes = new c_Fractal_Tree * [State_Node_Tree_Count];

		for (int cou_Index = 0; cou_Index < State_Node_Tree_Count; cou_Index++)
		{
			State_Nodes[cou_Index] = NULL;
		}

		for (int cou_Index = 0; cou_Index < (State_Node_Tree_Count - 1); cou_Index++)
		{
			State_Nodes[cou_Index] = tmp_State_Nodes[cou_Index];
			tmp_State_Nodes[cou_Index] = NULL;
		}

		if (tmp_State_Nodes != NULL) { delete[] tmp_State_Nodes; }
		tmp_State_Nodes = NULL;

		State_Nodes[State_Node_Tree_Count - 1] = new c_Fractal_Tree;

		return (State_Node_Tree_Count - 1);
	}

	//Creates a new node and adds it to the fractal tree.
	//Each node is stored as a link in a linked list.
	c_Node* new_Node()
	{
		//Create the node.
		*Current_Node = new c_Node;

		//Sets the new nodes ID.
		(*Current_Node)->NID = Node_Count;

		//Queries the node tree with the current NID to get the fractal node ready.
		Nodes.search(Node_Count);

		//Increment the node tracker.
		Node_Count++;

		//Sets the node in the node tree.
		Nodes.set_Current_Node_NAdd(*Current_Node);

		//Set the Current node to the next one in the chain.
		Current_Node = &(*Current_Node)->Next;

		//Return the created state node using the state tree as Current_Node is changed.
		return Nodes.get_Current_Node_NAdd();
	}

	//Creates a new node, then adds it to the state tree.
	//Assumes the construct is already registered so the index is valid.
	c_Node* new_State_Node(int p_Construct, uint64_t p_State)
	{
		c_Node* tmp_State_Node = new_Node();

		//Enter the state into the tree. The NNet node still needs linked to the pointer in the fractal node.
		State_Nodes[p_Construct]->search(p_State);

		//Assign the newly minted node in the fractal state tree.
		State_Nodes[p_Construct]->set_Current_Node_NAdd(tmp_State_Node);

		tmp_State_Node->State = p_State;

		tmp_State_Node->set_Type(0);

		return tmp_State_Node;
	}

	//This is used during loading when you have the node to bind to the state.
	void assign_State_Node(int p_State_Tree, c_Node * p_Node, uint64_t p_State)
	{
		//Enter the state into the tree. The NNet node still needs linked to the pointer in the fractal node.
		State_Nodes[p_State_Tree]->search(p_State);

		//Assign the newly minted node in the fractal state tree.
		State_Nodes[p_State_Tree]->set_Current_Node_NAdd(p_Node);

		p_Node->State = p_State;

		p_Node->set_Type(0);
	}

	//Creates a connection between nodes.
	//p_To forms dendritic connections to p_From, and on p_From you have the axonic connections.
	void create_Connections(c_Node* p_To, c_Node** p_From, int p_Count)
	{
		p_To->set_Dendrites(p_From, p_Count);

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			p_From[cou_Index]->add_Axon_Index(p_To, cou_Index);
		}
	}

	//Checks if an upper tier node exists.
	c_Node* does_Upper_Tier_Connection_Exist(c_Node** p_Legs, int p_Count)
	{
		//Search all the _F Axonic processes for the first leg, if found query the node to see if the leg permutations match.
		if (p_Legs[0] != NULL)
		{
			return p_Legs[0]->does_Upper_Tier_Connection_Exist(p_Legs, p_Count);
		}
		return NULL;
	}

	//Gets an upper tier node based on the given legs.
	c_Node* get_Upper_Tier_Node(c_Node** p_Legs, int p_Count, int p_Type, int p_Tier = 0)
	{
		if (p_Legs == NULL) { return NULL; }
		if (p_Count == 0) { return NULL; }

		c_Node* tmp_Node = NULL;

		//See if the node exists yet.
		tmp_Node = does_Upper_Tier_Connection_Exist(p_Legs, p_Count);

		/*
		std::cout << "\n DUTCE: " << tmp_Node;
		for (int cou_Leg = 0; cou_Leg < p_Count; cou_Leg++)
		{
			std::cout << "\n    [" << cou_Leg << "] " << p_Legs[cou_Leg];
		}
		*/

		//If the node doesn't exist then we create it, and then create the connection.
		if (tmp_Node == NULL)
		{
			//std::cout << "\n  New Node";

			tmp_Node = new_Node();

			create_Connections(tmp_Node, p_Legs, p_Count);

			tmp_Node->set_Type(p_Type);

			tmp_Node->Tier = p_Tier;
		}

		//std::cout << "\n  End: " << tmp_Node;
		return tmp_Node;
	}

	//If a state node exists in the given construct index then return it.
	//Otherwise return NULL.
	//This assumes the [Index] is valid
	c_Node* does_State_Node_Exist(int p_Index, uint64_t p_Data)
	{
		//Search the state tree give.
		State_Nodes[p_Index]->search(p_Data);
		if (State_Nodes[p_Index]->flg_Foundit)
		{
			return State_Nodes[p_Index]->get_Current_Node_NAdd();
		}
		return NULL;
	}

	//Checks to see if a node in the given assembly is bound to the given state, if not the node is created.
	c_Node* get_State_Node(int p_Index, uint64_t p_Data)
	{
		//See if the state node exists yet.
		c_Node* tmp_Node = does_State_Node_Exist(p_Index, p_Data);
		if (tmp_Node != NULL)
		{
			//If we found it then we return it.
			return tmp_Node;
		}

		//Create the new node and return it, new_State_Node handles the binding.
		return new_State_Node(p_Index, p_Data);
	}


	//Queries the node tree with the current NID to get the fractal node ready.
	c_Node* get_Node_Ref_By_NID(uint64_t p_NID)
	{

		//Queries the node tree with the given NID.
		Nodes.search(p_NID);

		if (Nodes.flg_Foundit)
		{
			return Nodes.get_Current_Node_NAdd();
		}
		return NULL;
	}

	//Gets the current fat row.
	uint64_t get_Fat_Tier()
	{
		return Fat_Tier;
	}

	//Sets the fat row.
	void set_Fat_Tier(uint64_t p_Fat_Tier)
	{
		Fat_Tier = p_Fat_Tier;
	}

	//Iterates through every node and outputs their bp_O()
	void output_BP(int p_Datatype = 0)
	{
		c_Node* tmp_Node;
		tmp_Node = Root;
		uint64_t tmp_Count = 0;
		while (tmp_Node != NULL)
		{
			std::cout << "\n<" << tmp_Count << ">";
			tmp_Node->bp_O(p_Datatype);

			tmp_Node = tmp_Node->Next;
			tmp_Count++;
		}
	}

	//Iterates through every node and outputs their bp_O()
	void output_Raw()
	{
		c_Node* tmp_Node;
		tmp_Node = Root;

		while (tmp_Node != NULL)
		{
			tmp_Node->output_Node_Raw();

			tmp_Node = tmp_Node->Next;
		}
	}

	//Finds given NID and outputs the bp_O()
	void output_BP_NID(uint64_t p_NID)
	{
		if (p_NID == 0)
		{
			std::cerr << "\n --== ERROR | Node[" << p_NID << "] is NULL node ==--";
			return;
		}

		c_Node* tmp_Node = NULL;
		tmp_Node = get_Node_Ref_By_NID(p_NID);
		if (tmp_Node != NULL)
		{
			std::cout << "\nNID: " << tmp_Node->NID << " ";
			std::cout << "Type: " << int(tmp_Node->Type) << " ";
			std::cout << "Symbol: ";
			tmp_Node->bp_O();
		}
		else
		{
			std::cerr << "\n --== ERROR | Node[" << p_NID << "] not found. ==--";
		}
	}

	//Finds given NID and outputs the bp_O()
	void output_Symbol_uint(uint64_t p_NID)
	{
		if (p_NID == 0)
		{
			std::cerr << "\n --== ERROR | Node[" << p_NID << "] is NULL node ==--";
			return;
		}

		c_Node* tmp_Node = NULL;
		tmp_Node = get_Node_Ref_By_NID(p_NID);
		if (tmp_Node != NULL)
		{
			std::cout << "\nNID: " << tmp_Node->NID << " ";
			std::cout << "Type: " << int(tmp_Node->Type) << " ";
			std::cout << "Symbol: ";
			tmp_Node->bp_O(1);
		}
		else
		{
			std::cerr << "\n --== ERROR | Node[" << p_NID << "] not found. ==--";
		}
	}


};