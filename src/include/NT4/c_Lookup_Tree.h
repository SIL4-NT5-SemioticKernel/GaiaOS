//This class forms the basic node on which the lookup tree is built. It's a basic binary tree.
class c_Lookup_Node
{
	friend class c_Lookup_Tree;
private:


public:

	//Legs of the node.
	c_Lookup_Node* Right;
	c_Lookup_Node* Left;

	//Node is also an LL
	c_Lookup_Node* Next;

	//The name bang tang in sophistic slang mang.
	std::string Name;

	//This stores the data, a uint64_t
	uint64_t Data;

	c_Lookup_Node()
	{
		Right = NULL;
		Left = NULL;

		Data = 0;
		
		Name = "NAMELESS & BLAMELESS";
	}

	~c_Lookup_Node()
	{
		Next = NULL;
		if (Right != NULL) { delete Right; Right = NULL; }
		if (Left != NULL) { delete Left; Left = NULL; }
	}

};

//The fractal state tree is a trinary tree that uses a bitshifted input to dtermine which nodes to assign.
class c_Lookup_Tree
{
private:


public:

	//The root node.
	c_Lookup_Node* Root;

	//the current node that will allow referencing outside of the search function.
	c_Lookup_Node** Current;

	//The current LL node.
	c_Lookup_Node** Current_LL;

	//the flag that set when a node is found already set
	bool flg_Foundit;

	c_Lookup_Tree()
	{
		Root = NULL;
		Current = NULL;
		Current_LL = &Root;
		flg_Foundit = false;
	}

	~c_Lookup_Tree()
	{
		if (Root != NULL) { delete Root; Root = NULL; }
	}
	
	//Resets the tree.
	void reset()
	{
		if (Root != NULL) { delete Root; Root = NULL; }
	}

	//Search for a state.
	void search(std::string p_Name)
	{
		//Set the flag to indicate that the data was not found.
		flg_Foundit = false;

		//The data is 64 bits deep. So start the shift at 63.
		query_Node(Root, p_Name);
	}

	//Queries a node. 
	//p_Node is the node to check against the query value.
	int query_Node(c_Lookup_Node*& p_Node, std::string p_Name)
	{
		//If the passed node is NULL create it.
		if (p_Node == NULL)
		{
			p_Node = new c_Lookup_Node;
			p_Node->Right = NULL;
			p_Node->Left = NULL;
			p_Node->Next = NULL;
			p_Node->Name = p_Name;

			if (Current_LL != NULL)
			{
				(*Current_LL)->Next = p_Node;
				Current_LL = &p_Node;
			}
			else
			{
				Current_LL = &p_Node;
			}

			//Set the current node to the newly created node.
			Current = &p_Node;

			//The node was not found, one had to be created.
			flg_Foundit = false;

			//Return as we are done here.
			return 0;
		}

		//If the node data matches the given data exactly the node has been found.
		//Set Current to the current node.
		//Set the flg_Foundit to true as the node was found without creating it.
		if (p_Node->Name == p_Name)
		{
			Current = &p_Node;
			flg_Foundit = true;
			return 1;
		}

		//If the tmp_Data is less than the nodes data then look on the left leg.
		if (p_Node->Name < p_Name)
		{
			query_Node(p_Node->Left, p_Name);
			return 0;
		}

		//If the tmp_Data is more than the nodes data then look on the right leg.
		if (p_Node->Name > p_Name)
		{
			query_Node(p_Node->Right, p_Name);
			return 0;
		}
		return 0;
	}

	//Outputs the tree starting at the root node.
	void output_Tree()
	{
		output_Node(Root, 0);
	}

	//Outputs a given node. p_Tab determines an offset during output.
	void output_Node(c_Lookup_Node * &p_Node, int p_Tab)
	{
		if (p_Node == NULL) { return; }
		output_Node(p_Node->Left, (p_Tab + 1));
		std::cout << "\n";
		for (int cou_Index = 0; cou_Index < p_Tab; cou_Index++)
		{
			std::cout << "  ";
		}
		std::cout << "Name: " << (p_Node->Name);
		std::cout << " - Data: ";
		std::cout << (p_Node->Data);
		output_Node(p_Node->Right, (p_Tab + 2));
	}

	//Retrieves the NAdd from a given node.
	uint64_t get_Data(c_Lookup_Node * &p_Node)
	{
		if (p_Node != NULL)
		{
			return p_Node->Data;
		}
		else 
		{
			return NULL;
		}
	}

	//Sets the current node data to the given data.
	void set_Data(c_Lookup_Node*& p_Node, uint64_t p_Data)
	{
		if (p_Node != NULL)
		{
			p_Node->Data = p_Data;
		}
		else
		{
			std::cerr << "\n\n   !(o,O)?   Error when setting node data in lookup tree. No current node, flg_Foundit false, passed node NULL.";
		}
	}

	//Sets the current nodes NAdd.
	void set_Current_Data(uint64_t p_Data)
	{
		set_Data(*Current, p_Data);
	}

	//Gets the current nodes NAdd.
	uint64_t get_Current_Data()
	{
		return get_Data(*Current);
	}
};
