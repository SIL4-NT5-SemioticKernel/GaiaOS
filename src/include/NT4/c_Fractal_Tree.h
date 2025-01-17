


/*
		When I was building the networks I ran into the problem of searching them for existing state nodes. A state
	node is the node that associates an input with a node. State nodes allows for storing information in the networks and
	subsequently retrieving information from them. With the higher networks that used node IDs (NAdd)s from the lower
	constructs this became an issue because the number of unique nodes input into the network could get into the millions.
		In building a network you have to first take an input and search the network to see if that state was already
	associated with a node. Originally this was done with binary trees. The millions of unique inputs into these binary
	trees caused long search times. When it came to reading the nodes in there was another issue with the NAdds being
	sequential leading to extremely long branches in the binary tree that stored almost all of the nodes. This led to
	stack corruption with recusive searches.
		The first thing done was changing the recursive searches to an iterative one. This solved the issue of stack
	corruption but did nothing to solve the long search times. So then an optimization function was created which
	redistributed the nodes evenly along the branches. This function helped but did not fix the issue as the optimization
	had to be done frequently and was costly in and of itself.
		In the end I used a trinary tree combined with bit shifting the query. This allowed for a tree that has branches
	with fixed lengths. With the branches not getting excessive it does not run into the problem of stack corruptions
	even with millions of entries.
*/


//This class forms the basic node on which the fractal tree is built.
class c_Fractal_Node
{
	friend class c_Fractal_Tree;
private:

	//Legs of the node.
	c_Fractal_Node* Right;
	c_Fractal_Node* Center;
	c_Fractal_Node* Left;

public:

	//This stores the state||node ID
	uint64_t Data;

	//Pointer to the actual node.
	c_Node* NAdd;

	c_Fractal_Node()
	{
		Right = NULL;
		Center = NULL;
		Left = NULL;
		Data = 0;
		NAdd = NULL;
	}

	~c_Fractal_Node()
	{
		NAdd = NULL;
		if (Right != NULL) { delete Right; Right = NULL; }
		if (Center != NULL) { delete Center; Center = NULL; }
		if (Left != NULL) { delete Left; Left = NULL; }
	}

};

//The fractal state tree is a trinary tree that uses a bitshifted input to dtermine which nodes to assign.
class c_Fractal_Tree
{
private:


public:

	//The root node.
	c_Fractal_Node* Root;

	//the current node that will allow referencing outside of the search function.
	c_Fractal_Node** Current;

	//the flag that set when a node is found already set
	bool flg_Foundit;

	//Holds the bitshifted data to compare to the nodes data.
	uint64_t tmp_Data;

	//This is used for saving and loading, tracks the number of nodes in the tree so the system knows how many nodes to expect during loading.
	int State_Count;

	c_Fractal_Tree()
	{
		Root = NULL;
		Current = NULL;
		flg_Foundit = false;

		State_Count = 0;

		tmp_Data = 0;
	}

	~c_Fractal_Tree()
	{
		if (Root != NULL) { delete Root; Root = NULL; }
	}
	
	//Resets the tree.
	void reset()
	{
		if (Root != NULL) { delete Root; Root = NULL; }
	}

	//Search for a state.
	void search(uint64_t p_Data)
	{
		//Set the flag to indicate that the data was not found.
		flg_Foundit = false;

		tmp_Data = 0;

		//The data is 64 bits deep. So start the shift at 63.
		query_Node(Root, p_Data, 63);
	}

	//Queries a node. 
	//p_Node is the node to check against the query value.
	//p_Data is the data that is checked against.
	//p_Shift is the number of bits to extract for comparison.
	int query_Node(c_Fractal_Node*& p_Node, uint64_t p_Data, int p_Shift)
	{
		//Shift the data to get the temporary value.
		tmp_Data = p_Data >> p_Shift;

		//If the passed node is NULL create it.
		if (p_Node == NULL)
		{
			p_Node = new c_Fractal_Node;
			p_Node->Right = NULL;
			p_Node->Center = NULL;
			p_Node->Left = NULL;
			p_Node->NAdd = NULL;

			if (p_Shift >= 1)
			{
				//If the query is not at the end set the data to tmp_Data rather than p_Data.
				p_Node->Data = tmp_Data;
			}
			else
			{
				//If the query has reached the end set the data to the full query data.
				p_Node->Data = p_Data;

				//Set the current node to the newly created node.
				Current = &p_Node;

				//The node was not found, one had to be created.
				flg_Foundit = false;

				//We have registered a new leaf.
				State_Count++;

				//Return as we are done here.
				return 0;
			}
		}

		//If the current data matches the tmp data then another depth is explored.
		if (p_Node->Data == tmp_Data && p_Shift >= 1)
		{
			if (p_Shift == 1) { query_Node(p_Node->Center, p_Data, 0); return 0; }
			query_Node(p_Node->Center, p_Data, (p_Shift - 2));
			return 0;
		}

		//If the node data matches the given data exactly the node has been found.
		//Set Current to the current node.
		//Set the flg_Foundit to true as the node was found without creating it.
		if (p_Node->Data == p_Data)
		{
			Current = &p_Node;
			if (p_Node->NAdd != NULL) { flg_Foundit = true; }
			return 0;
		}

		//If the tmp_Data is less than the nodes data then look on the left leg.
		if (tmp_Data < p_Node->Data)
		{
			query_Node(p_Node->Left, p_Data, p_Shift);
			return 0;
		}

		//If the tmp_Data is more than the nodes data then look on the right leg.
		if (tmp_Data > p_Node->Data)
		{
			query_Node(p_Node->Right, p_Data, p_Shift);
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
	void output_Node(c_Fractal_Node * &p_Node, int p_Tab)
	{
		if (p_Node == NULL) { return; }
		output_Node(p_Node->Left, (p_Tab));
		std::cout << "\n";
		for (int cou_Index = 0; cou_Index < p_Tab; cou_Index++)
		{
			std::cout << "  ";
		}
		std::cout << (p_Node->Data) << "->";
		if (p_Node->NAdd != NULL) 
		{ 
			std::cout << p_Node->NAdd; 
			std::cout << "\n";
			(p_Node->NAdd)->output_Node_Raw();
		}
		else { std::cout << "NULL"; }
		output_Node(p_Node->Center, (p_Tab + 1));
		output_Node(p_Node->Right, (p_Tab));
	}

	//Outputs the tree with the backpropagation from the nodes starting at the root node.
	void output_Tree_BP()
	{
		std::cout << "\n\t Fractal Tree Backpropagation Station";
		output_Node_BP(Root);
	}

	//Outputs a given node and backpropagates the node it is linked to.
	void output_Node_BP(c_Fractal_Node * &p_Node)
	{
		if (p_Node == NULL) { return; }
		output_Node_BP(p_Node->Left);
		if ((p_Node->NAdd) != NULL) { std::cout << "\n\t" << (p_Node->Data) << "->" << (p_Node->NAdd); (p_Node->NAdd)->bp_O(); }
		output_Node_BP(p_Node->Center);
		output_Node_BP(p_Node->Right);
	}

	//Retrieves the NAdd from a given node.
	c_Node* get_Node_NAdd(c_Fractal_Node * &p_Node)
	{
		if (p_Node != NULL)
		{
			return p_Node->NAdd;
		}
		else {
			return NULL;
		}
	}

	//Sets a node NAdd.
	void set_Node_NAdd(c_Fractal_Node * &p_Node, c_Node * p_NAdd)
	{
		if (p_Node != NULL)
		{
			p_Node->NAdd = p_NAdd;
		}
	}

	//Sets the current nodes NAdd.
	c_Node* set_Current_Node_NAdd(c_Node * p_NAdd)
	{
		set_Node_NAdd(*Current, p_NAdd);
		return p_NAdd;
	}

	//Gets the current nodes NAdd.
	c_Node* get_Current_Node_NAdd()
	{
		return get_Node_NAdd(*Current);
	}

	//Saves the tree as a list of nodes and the state associated.
	void save_Tree(std::ofstream* p_SF)
	{
		*p_SF << "\n" << State_Count;
		save_Tree_Node(Root, p_SF);
	}

	//Outputs a given node and backpropagates the node it is linked to.
	void save_Tree_Node(c_Fractal_Node * &p_Node, std::ofstream* p_SF)
	{
		if (p_Node == NULL) { return; }
		save_Tree_Node(p_Node->Left, p_SF);
		
		if ((p_Node->NAdd) != NULL) 
		{ 
			*p_SF << "\n " << (p_Node->NAdd)->NID << " " << (p_Node->NAdd)->State;
		}

		save_Tree_Node(p_Node->Center, p_SF);
		save_Tree_Node(p_Node->Right, p_SF);
	}

};
