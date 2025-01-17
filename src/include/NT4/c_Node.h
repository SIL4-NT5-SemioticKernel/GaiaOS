//The state is stored as uint64_t so you will need to typecast to appropriate datatype.

//Quick n dirty to output the double for the node states
union u_Tmp
{
    double D;
    uint64_t U;
};

class c_Node
{
public:

    //The Node ID, used because the raw address may change upon saving/loading and external hooks need a solid grasp.
    uint64_t NID;

    //The next node in the network.
    c_Node* Next;

    //Axons and the counters. Axons[0][] are the ones connected to the first leg, used in finding UTN
    //Axons are organized by the dendritic index they are connected to, if they connect to leg [0] then they are an axon on hillock [0].
    c_Node*** Axons;
    int* Axon_Count;
    int Axon_Hillock_Count;

    //Dendrites, lower connections, leg order is of highest importance.
    //Each one has a weight, this is used in the pyramidal node networks and others with many layers to address the double legged node problem.
    c_Node** Dendrites;
    float* Dendrite_Weights;
    int Dendrite_Count;

    //The reinforcement counter.
    //A filter function will be built into the c_Neural_Network that applies:
    // exponential growth to reduce new trace impact, one offs and flukes get filtered
    // Sigmoid curve for the top end to prevent extreme bias in highly reinforced nodes.
    float RC;

    //The quanta bound to the node.
    uint64_t State;

    //This tracks the node type.
    //(0: NULL), (1: State), (2: Branch), (3: Treetop), (4: State Treetop)
    short int Type;

    float Charge;

    int Tier;
    int Index;

    c_Node()
    {
        NID = 0;

        //The next node.
        Next = NULL;

        //Axons and their counters.

        Axons = NULL;
        Axon_Count = 0;
        Axon_Hillock_Count = 0;

        //Dendrites.
        Dendrites = NULL;
        Dendrite_Weights = NULL;
        Dendrite_Count = 0;

        Tier = 0;
        Index = 0;

        //Reinforcement Counters.
        RC = 1.0;

        //The nodes state.
        State = 0;

        //The type is 0 to start with.
        Type = 0;

        Charge = 0.0;
    }

    //(0: State), (1: Branch), (2: Treetop), (3: State/Treetop)
    void set_Type(int p_Type)
    {
        Type = p_Type;
    }

    void resize_Axon_Hillocks(int p_Count)
    {
        c_Node*** tmp_Axons;
        int* tmp_Axon_Count;
        tmp_Axons = new c_Node ** [Axon_Hillock_Count];
        tmp_Axon_Count = new int [Axon_Hillock_Count];

        for (int cou_A = 0; cou_A < Axon_Hillock_Count; cou_A++)
        {
            tmp_Axons[cou_A] = Axons[cou_A];
            Axons[cou_A] = NULL;

            tmp_Axon_Count[cou_A] = Axon_Count[cou_A];
        }

        delete[] Axons;
        Axons = NULL;

        delete[] Axon_Count;
        Axon_Count = NULL;

        Axons = new c_Node ** [p_Count];
        Axon_Count = new int [p_Count];

        for (int cou_A = 0; cou_A < Axon_Hillock_Count; cou_A++)
        {
            Axons[cou_A] = tmp_Axons[cou_A];
            tmp_Axons[cou_A] = NULL;

            Axon_Count[cou_A] = tmp_Axon_Count[cou_A];
        }

        delete[] tmp_Axons;
        tmp_Axons = NULL;

        delete[] tmp_Axon_Count;
        tmp_Axon_Count = NULL;

        for (int cou_A = Axon_Hillock_Count; cou_A < p_Count; cou_A++)
        {
            Axons[cou_A] = NULL;
            Axon_Count[cou_A] = 0;
        }

        Axon_Hillock_Count = p_Count;
    }

    //Adds an axon to the axon list at the given index, if the index doesn't exist then exist it with resize_Axon_Hillocks()
    void add_Axon_Index(c_Node* p_Axon, int p_Index)
    {
        if (p_Index >= Axon_Hillock_Count) { resize_Axon_Hillocks(p_Index + 1); }

        c_Node** tmp_Axons;
        tmp_Axons = new c_Node * [Axon_Count[p_Index]];
        for (int cou_A = 0; cou_A < Axon_Count[p_Index]; cou_A++)
        {
            tmp_Axons[cou_A] = Axons[p_Index][cou_A];
            Axons[p_Index][cou_A] = NULL;
        }

        delete[] Axons[p_Index];
        Axons[p_Index] = NULL;

        Axons[p_Index] = new c_Node * [Axon_Count[p_Index] + 1];

        for (int cou_A = 0; cou_A < Axon_Count[p_Index]; cou_A++)
        {
            Axons[p_Index][cou_A] = tmp_Axons[cou_A];
            tmp_Axons[cou_A] = NULL;
        }
        delete[] tmp_Axons;
        tmp_Axons = NULL;

        Axons[p_Index][Axon_Count[p_Index]] = p_Axon;
        Axon_Count[p_Index]++;
    }

    //This checks for double legged nodes and adjusts the weights so the network doesn't favor repeating patterns.
    //The double legged charging issue isn't limited to 2 legs, but that is the nomenclature that developed before the higher dimensional networks were explored.
    void rectify_Double_Legged_Nodes()
    {
        float tmp_Count = 0;

        //Step through each leg, comparing to the other legs, and then taking dividing the weight by the number of same legs, 3 same legs = 1/3 for each of the three resulting in 3 with .3333 you see?
        for (int cou_D = 0; cou_D < Dendrite_Count - 1; cou_D++)
        {
            tmp_Count = 1;

            if (Dendrite_Weights[cou_D] != 1.0) { continue; }

            for (int cou_XD = (cou_D + 1); cou_XD < Dendrite_Count; cou_XD++)
            {
                if (Dendrites[cou_D] == Dendrites[cou_XD]) { tmp_Count++; }
            }

            if (tmp_Count > 1)
            {
                Dendrite_Weights[cou_D] = 1 / tmp_Count;

                for (int cou_XD = (cou_D + 1); cou_XD < Dendrite_Count; cou_XD++)
                {
                    if (Dendrites[cou_D] == Dendrites[cou_XD]) { Dendrite_Weights[cou_XD] = 1 / tmp_Count; }
                }
            }
        }
    }

    //Sets the dendrites of the node.
    //This assumes the node has no dendrites yet, if it does you be dangling and jangling
    //It checks to see if any legs match, if they do their charges are cut proportionatly. 
    void set_Dendrites(c_Node** p_Dendrites, int p_Count)
    {
        Dendrites = new c_Node * [p_Count];
        Dendrite_Weights = new float[p_Count];

        for (int cou_D = 0; cou_D < p_Count; cou_D++)
        {
            Dendrites[cou_D] = p_Dendrites[cou_D];

            Dendrite_Weights[cou_D] = 1.0;
        }

        Dendrite_Count = p_Count;
    }

    //Searches the axons to see if an upper tier connection exists.
    //This is always called from the first leg, that is why we separate _F from normal.
    c_Node* does_Upper_Tier_Connection_Exist(c_Node** p_Nodes, int p_Count)
    {
        if (Axon_Hillock_Count > 0)
        {
            //std::cout << "\n Axon_Count[0]: " << Axon_Count[0];

            //Check the axons on the right side for the 
            for (int cou_A = 0; cou_A < Axon_Count[0]; cou_A++)
            {
                //std::cout << "\n   Axon_F[" << cou_A << "] " << Axons[0][cou_A];
                if (Axons[0][cou_A]->does_Lower_Connection_Exist(p_Nodes, p_Count))
                {
                    //std::cout << "\n >>>>>>>>>>>>>Foundit: " << Axons[0][cou_A];
                    return Axons[0][cou_A];
                }
            }
        }
        return NULL;
    }

    //Checks if the given node matches a dendrite on the right leg.
    bool does_Lower_Connection_Exist(c_Node ** p_Nodes, int p_Count)
    {
        if (Dendrite_Count != p_Count) { return 0; }

        //Check the lower connections to see if they match.
        for (int cou_C = 1; cou_C < p_Count; cou_C++)
        {
            if (Dendrites[cou_C] != p_Nodes[cou_C]) { return 0; }
        }
        return 1;
    }

    //Binds a node to a quanta of data, the state of the input.
    void bind_State(uint64_t p_State)
    {
        State = p_State;
    }

    //Initiates a backpropagation that outputs the pattern represented by this node.
    //p_Datatype: 0: String, 1: uint64_t
    void bp_O(int p_Datatype = 0)
    {
        //---std::cout << "<=- NID " << NID << " Pat [ ";
        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            if (Dendrites[0] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[0]->Type != 2) && (Dendrites[0]->Type != 3))
                {
                    Dendrites[0]->bp_F(p_Datatype);
                }
                //If the dendritically connected node is a treetop then call the _O on it.
                if ((Dendrites[0]->Type == 2) || (Dendrites[0]->Type == 3))
                {
                    Dendrites[0]->bp_O();
                }
            }

            for (int cou_D = 1; cou_D < Dendrite_Count; cou_D++)
            {
                if (Dendrites[cou_D] != NULL)
                {
                    //If not a treetop then call bp_M()
                    if ((Dendrites[cou_D]->Type != 2) && (Dendrites[cou_D]->Type != 3))
                    {
                        Dendrites[cou_D]->bp_M(p_Datatype);
                        continue;
                    }

                    //If the dendritically connected node is a treetop then call the _O on it.
                    if ((Dendrites[cou_D]->Type == 2) || (Dendrites[cou_D]->Type == 3))
                    {
                        Dendrites[cou_D]->bp_O(p_Datatype);
                    }
                }
            }
        }
        else
        {
            //Output the state
            //std::cout << " <" << NID << " :: " << State << "> ";

            //u_Tmp tmp_State; tmp_State.U = State;
            //std::cout << " [ " << tmp_State.D << ", " << State << " ] ";
            if (p_Datatype == 0) { std::cout << char(State); }
            if (p_Datatype == 1) { std::cout << State; }
            u_Data tmp_Bit; tmp_Bit.U = State;
            if (p_Datatype == 2) { std::cout << tmp_Bit.D; }
        }
        //---std::cout << " ] -=>";
    }

    //bp_Output the left node.
    void bp_F(int p_Datatype)
    {
        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            if (Dendrites[0] != NULL) { Dendrites[0]->bp_F(p_Datatype); }
            for (int cou_D = 1; cou_D < Dendrite_Count; cou_D++)
            {
                if (Dendrites[cou_D] != NULL) 
                {
                    //If the dendritically connected node is a treetop then call the _O on it.
                    if ((Dendrites[cou_D]->Type == 2) || (Dendrites[cou_D]->Type == 3))
                    {
                        Dendrites[cou_D]->bp_O(p_Datatype);
                    }
                    else
                    {
                        Dendrites[cou_D]->bp_M(p_Datatype);
                    }
                }

            }
        }
        else
        {
            //Output the state
            //std::cout << " <" << NID << " :: " << State << "> ";
            //u_Tmp tmp_State; tmp_State.U = State;
            //std::cout << " [ " << tmp_State.D << ", " << State << " ] ";
            if (p_Datatype == 0) { std::cout << char(State); }
            if (p_Datatype == 1) { std::cout << State; }
            u_Data tmp_Bit; tmp_Bit.U = State;
            if (p_Datatype == 2) { std::cout << tmp_Bit.D; }
        }
    }

    //bp_Output the other nodes, M stands for many.
    void bp_M(int p_Datatype)
    {
        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            for (int cou_D = 1; cou_D < Dendrite_Count; cou_D++)
            {
                if (Dendrites[cou_D] != NULL)
                {
                    //If the dendritically connected node is a treetop then call the _O on it.
                    if ((Dendrites[cou_D]->Type == 2) || (Dendrites[cou_D]->Type == 3))
                    {
                        Dendrites[cou_D]->bp_O(p_Datatype);
                    }
                    else
                    {
                        Dendrites[cou_D]->bp_M(p_Datatype);
                    }
                }
            }
        }
        else
        {
            //Output the state
            //std::cout << " <" << NID << " :: " << State << "> ";
            //u_Tmp tmp_State; tmp_State.U = State;
            //std::cout << " [ " << tmp_State.D << ", " << State << " ] ";
            if (p_Datatype == 0) { std::cout << char(State); }
            if (p_Datatype == 1) { std::cout << " " << State; }
            u_Data tmp_Bit; tmp_Bit.U = State;
            if (p_Datatype == 2) { std::cout << tmp_Bit.D; }
        }
    }

    


    //Initiates a backpropagation that 
    void bp_Trace_O(c_Linked_List_Handler * p_LL)
    {
        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            //---std::cout << "*";
            if (Dendrites[0] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[0]->Type != 2) && (Dendrites[0]->Type != 3))
                {
                    Dendrites[0]->bp_Trace_F(p_LL);
                }

                //If the dendritically connected node is a treetop then call the _O on it.
                if ((Dendrites[0]->Type == 2) || (Dendrites[0]->Type == 3))
                {
                    Dendrites[0]->bp_Trace_O(p_LL);
                }
            }

            for (int cou_D = 1; cou_D < Dendrite_Count; cou_D++)
            {
                if (Dendrites[cou_D] != NULL)
                {
                    //If not a treetop then call bp_M()
                    if ((Dendrites[cou_D]->Type != 2) && (Dendrites[cou_D]->Type != 3))
                    {
                        Dendrites[cou_D]->bp_Trace_M(p_LL);
                        continue;
                    }

                    //If the dendritically connected node is a treetop then call the bp_O on it.
                    if ((Dendrites[cou_D]->Type == 2) || (Dendrites[cou_D]->Type == 3))
                    {
                        Dendrites[cou_D]->bp_Trace_O(p_LL);
                    }
                }
            }
        }
        else
        {
            //Output the state
            std::cout << char(State);
            p_LL->new_LL(State);
        }
    }

    //bp_Output the left node.
    void bp_Trace_F(c_Linked_List_Handler* p_LL)
    {
        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            if (Dendrites[0] != NULL) { Dendrites[0]->bp_Trace_F(p_LL); }
            for (int cou_D = 1; cou_D < Dendrite_Count; cou_D++)
            {
                if (Dendrites[cou_D] != NULL) 
                {
                    Dendrites[cou_D]->bp_Trace_M(p_LL);
                }
            }
        }
        else
        {
            //Output the state
            //---std::cout << " <" << NID << " :: " << State << "> ";
            p_LL->new_LL(State);
        }
    }

    //bp_Output the other nodes, M stands for many.
    void bp_Trace_M(c_Linked_List_Handler* p_LL)
    {
        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            for (int cou_D = 1; cou_D < Dendrite_Count; cou_D++)
            {
                if (Dendrites[cou_D] != NULL)
                {
                    Dendrites[cou_D]->bp_Trace_M(p_LL);
                }
            }
        }
        else
        {
            //Output the state
            //---std::cout << " <" << NID << " :: " << State << "> ";
            p_LL->new_LL(State);
        }
    }


    //Initiates a backpropagation that 
    //If this is the first node then it starts at 0,0. Each step down the tree it tracks the direction and adjust the x,y going down according to a pre-determined matrix applied to it.
    //Get the legs for the node, the 2x2
    // 
    //tmp_Nodes[0] = Scaffold[cou_T - 1][cou_X][cou_Y][cou_Z];
    //tmp_Nodes[1] = Scaffold[cou_T - 1][cou_X + 1][cou_Y][cou_Z];
    //tmp_Nodes[2] = Scaffold[cou_T - 1][cou_X][cou_Y + 1][cou_Z];
    //tmp_Nodes[3] = Scaffold[cou_T - 1][cou_X + 1][cou_Y + 1][cou_Z];

    //tmp_Nodes[4] = Scaffold[cou_T - 1][cou_X][cou_Y][cou_Z + 1];
    //tmp_Nodes[5] = Scaffold[cou_T - 1][cou_X + 1][cou_Y][cou_Z + 1];
    //tmp_Nodes[6] = Scaffold[cou_T - 1][cou_X][cou_Y + 1][cou_Z + 1];
    //tmp_Nodes[7] = Scaffold[cou_T - 1][cou_X + 1][cou_Y + 1][cou_Z + 1];
    void bp_3D_Trace_O(c_Linked_List_Handler * p_LL, c_Linked_List_Handler * p_LL_X, c_Linked_List_Handler * p_LL_Y, c_Linked_List_Handler * p_LL_Z, uint64_t p_X = 0, uint64_t p_Y = 0, uint64_t p_Z = 0)
    {
        //---std::cout << "\n bp_3D_Trace_O <=- " << NID << " " << p_X << " " << p_Y << " " << p_Z;
        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            //---std::cout << "*";
            if (Dendrites[0] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[0]->Type != 2) && (Dendrites[0]->Type != 3))
                {
                    Dendrites[0]->bp_3D_Trace_F(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X, p_Y, p_Z);
                }
            }
            bp_3D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X, p_Y, p_Z);
        }
        else
        {
            //Output the state
            //---std::cout << " <" << NID << " :: " << State << "> ";
            p_LL->new_LL(State);
            p_LL_X->new_LL(p_X);
            p_LL_Y->new_LL(p_Y);
            p_LL_Z->new_LL(p_Z);
        }
        //---std::cout << " -=>";
    }

    //bp_Output the left node.
    void bp_3D_Trace_F(c_Linked_List_Handler* p_LL, c_Linked_List_Handler* p_LL_X, c_Linked_List_Handler* p_LL_Y, c_Linked_List_Handler* p_LL_Z, uint64_t p_X = 0, uint64_t p_Y = 0, uint64_t p_Z = 0)
    {

        //---std::cerr << "\n bp_3D_Trace_F <=- " << NID << " " << p_X << " " << p_Y << " " << p_Z;

        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        //tmp_Nodes[0] = Scaffold[cou_T - 1][cou_X][cou_Y][cou_Z]; //1
        if (Dendrite_Count != 0)
        {
            //---std::cout << "*";
            if (Dendrites[0] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[0]->Type != 2) && (Dendrites[0]->Type != 3))
                {
                    Dendrites[0]->bp_3D_Trace_F(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X, p_Y, p_Z);
                }
            }
            bp_3D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X, p_Y, p_Z);
        }
        else
        {
            //Output the state
            //---std::cout << " <" << NID << " :: " << State << "> ";
            p_LL->new_LL(State);
            p_LL_X->new_LL(p_X);
            p_LL_Y->new_LL(p_Y);
            p_LL_Z->new_LL(p_Z);
        }
        //---std::cout << " -=>";
    }

    //bp_Output the other nodes, M stands for many.
    void bp_3D_Trace_M(c_Linked_List_Handler* p_LL, c_Linked_List_Handler* p_LL_X, c_Linked_List_Handler* p_LL_Y, c_Linked_List_Handler* p_LL_Z, uint64_t p_X = 0, uint64_t p_Y = 0, uint64_t p_Z = 0)
    {
        //---std::cout << "\n bp_3D_Trace_M <=- " << NID << " " << p_X << " " << p_Y << " " << p_Z;

        if (Dendrite_Count != 0)
        {
            //I'm doing these by hand so hopefully it is more approachable to newbs, if not we can always refactor with a nicer way, this looks like shit, but it is readable.


            //Scaffold[cou_T - 1][cou_X + 1][cou_Y][cou_Z];
            if (Dendrite_Count > 1)
            {
                if (Dendrites[1] != NULL)
                {
                    if ((Dendrites[1]->Type != 2) && (Dendrites[1]->Type != 3))
                    {
                        Dendrites[1]->bp_3D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X + 1, p_Y, p_Z);
                    }
                }
            }

            //Scaffold[cou_T - 1][cou_X + 1][cou_Y][cou_Z];
            if (Dendrite_Count > 2)
            {
                if (Dendrites[2] != NULL)
                {
                    if ((Dendrites[2]->Type != 2) && (Dendrites[2]->Type != 3))
                    {
                        Dendrites[2]->bp_3D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X, p_Y + 1, p_Z);
                    }
                }
            }


            //Scaffold[cou_T - 1][cou_X + 1][cou_Y + 1][cou_Z];
            if (Dendrite_Count > 3)
            {
                if (Dendrites[3] != NULL)
                {
                    if ((Dendrites[3]->Type != 2) && (Dendrites[3]->Type != 3))
                    {
                        Dendrites[3]->bp_3D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X + 1, p_Y + 1, p_Z);
                    }
                }
            }

            //Scaffold[cou_T - 1][cou_X][cou_Y][cou_Z + 1];
            if (Dendrite_Count > 4)
            {
                if (Dendrites[4] != NULL)
                {
                    if ((Dendrites[4]->Type != 2) && (Dendrites[4]->Type != 3))
                    {
                        Dendrites[4]->bp_3D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X, p_Y, p_Z + 1);
                    }
                }
            }

            //Scaffold[cou_T - 1][cou_X + 1][cou_Y][cou_Z + 1];
            if (Dendrite_Count > 5)
            {
                if (Dendrites[5] != NULL)
                {
                    if ((Dendrites[5]->Type != 2) && (Dendrites[5]->Type != 3))
                    {
                        Dendrites[5]->bp_3D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X + 1, p_Y, p_Z + 1);
                    }
                }
            }

            //Scaffold[cou_T - 1][cou_X][cou_Y + 1][cou_Z + 1];
            if (Dendrite_Count > 6)
            {
                if (Dendrites[6] != NULL)
                {
                    if ((Dendrites[6]->Type != 2) && (Dendrites[6]->Type != 3))
                    {
                        Dendrites[6]->bp_3D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X, p_Y + 1, p_Z + 1);
                    }
                }
            }

            //Scaffold[cou_T - 1][cou_X + 1][cou_Y + 1][cou_Z + 1];
            if (Dendrite_Count > 7)
            {
                if (Dendrites[7] != NULL)
                {
                    if ((Dendrites[7]->Type != 2) && (Dendrites[7]->Type != 3))
                    {
                        Dendrites[7]->bp_3D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_LL_Z, p_X + 1, p_Y + 1, p_Z + 1);
                    }

                }
            }
        }
        else
        {
            //Output the state
            //---std::cout << " <" << NID << " :: " << State << "> ";
            p_LL->new_LL(State);
            p_LL_X->new_LL(p_X);
            p_LL_Y->new_LL(p_Y);
            p_LL_Z->new_LL(p_Z);
        }
    }


    //Initiates a backpropagation that 
    //If this is the first node then it starts at 0,0. Each step down the tree it tracks the direction and adjust the x,y going down according to a pre-determined matrix applied to it.
    //Get the legs for the node, the 2x2
    //tmp_Nodes[0] = Scaffold[cou_T - 1][cou_X][cou_Y]; //1
    //tmp_Nodes[1] = Scaffold[cou_T - 1][cou_X][cou_Y + 1]; //2
    //tmp_Nodes[2] = Scaffold[cou_T - 1][cou_X + 1][cou_Y]; //3
    //tmp_Nodes[3] = Scaffold[cou_T - 1][cou_X + 1][cou_Y + 1]; //4
    void bp_2D_Trace_O(c_Linked_List_Handler * p_LL, c_Linked_List_Handler * p_LL_X, c_Linked_List_Handler * p_LL_Y, uint64_t p_X = 0, uint64_t p_Y = 0)
    {
        //---std::cout << "\n bp_2D_Trace_O <=- " << NID << " " << p_X << " " << p_Y;
        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            //---std::cout << "*";
            if (Dendrites[0] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[0]->Type != 2) && (Dendrites[0]->Type != 3))
                {
                    Dendrites[0]->bp_2D_Trace_F(p_LL, p_LL_X, p_LL_Y, p_X, p_Y);
                }

                //If the dendritically connected node is a treetop then call the _O on it.
                if ((Dendrites[0]->Type == 2) || (Dendrites[0]->Type == 3))
                {
                    Dendrites[0]->bp_2D_Trace_O(p_LL, p_LL_X, p_LL_Y, p_X, p_Y);
                }
            }

            //---std::cout << "*";
            if (Dendrites[1] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[1]->Type != 2) && (Dendrites[1]->Type != 3))
                {
                    Dendrites[1]->bp_2D_Trace_F(p_LL, p_LL_X, p_LL_Y, p_X, p_Y + 1);
                }

                //If the dendritically connected node is a treetop then call the _O on it.
                if ((Dendrites[1]->Type == 2) || (Dendrites[1]->Type == 3))
                {
                    Dendrites[1]->bp_2D_Trace_O(p_LL, p_LL_X, p_LL_Y, p_X, p_Y + 1);
                }
            }

            //---std::cout << "*";
            if (Dendrites[2] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[2]->Type != 2) && (Dendrites[2]->Type != 3))
                {
                    Dendrites[2]->bp_2D_Trace_F(p_LL, p_LL_X, p_LL_Y, p_X + 1, p_Y);
                }

                //If the dendritically connected node is a treetop then call the _O on it.
                if ((Dendrites[2]->Type == 2) || (Dendrites[2]->Type == 3))
                {
                    Dendrites[2]->bp_2D_Trace_O(p_LL, p_LL_X, p_LL_Y, p_X + 1, p_Y);
                }
            }
            //---std::cout << "*";
            if (Dendrites[3] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[3]->Type != 2) && (Dendrites[3]->Type != 3))
                {
                    Dendrites[3]->bp_2D_Trace_F(p_LL, p_LL_X, p_LL_Y, p_X + 1, p_Y + 1);
                }

                //If the dendritically connected node is a treetop then call the _O on it.
                if ((Dendrites[3]->Type == 2) || (Dendrites[3]->Type == 3))
                {
                    Dendrites[3]->bp_2D_Trace_O(p_LL, p_LL_X, p_LL_Y, p_X + 1, p_Y + 1);
                }
            }

        }
        else
        {
            //Output the state
            //---std::cout << " <" << NID << " :: " << State << "> ";
            p_LL->new_LL(State);
            p_LL_X->new_LL(p_X);
            p_LL_Y->new_LL(p_Y);
        }
        //---std::cout << " -=>";
    }

    //bp_Output the left node.
    void bp_2D_Trace_F(c_Linked_List_Handler* p_LL, c_Linked_List_Handler* p_LL_X, c_Linked_List_Handler* p_LL_Y, uint64_t p_X = 0, uint64_t p_Y = 0)
    {

        //---std::cout << "\n bp_2D_Trace_F <=- " << NID << " " << p_X << " " << p_Y;

        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            //---std::cout << "*";
            if (Dendrites[0] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[0]->Type != 2) && (Dendrites[0]->Type != 3))
                {
                    Dendrites[0]->bp_2D_Trace_F(p_LL, p_LL_X, p_LL_Y, p_X, p_Y);
                }
            }

            //---std::cout << "*";
            if (Dendrites[1] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[1]->Type != 2) && (Dendrites[1]->Type != 3))
                {
                    Dendrites[1]->bp_2D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_X, p_Y + 1);
                }
            }

            //---std::cout << "*";
            if (Dendrites[2] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[2]->Type != 2) && (Dendrites[2]->Type != 3))
                {
                    Dendrites[2]->bp_2D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_X + 1, p_Y);
                }
            }
            //---std::cout << "*";
            if (Dendrites[3] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[3]->Type != 2) && (Dendrites[3]->Type != 3))
                {
                    Dendrites[3]->bp_2D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_X + 1, p_Y + 1);
                }
            }

        }
        else
        {
            //Output the state
            //---std::cout << " <" << NID << " :: " << State << "> ";
            p_LL->new_LL(State);
            p_LL_X->new_LL(p_X);
            p_LL_Y->new_LL(p_Y);
        }
    }

    //bp_Output the other nodes, M stands for many.
    void bp_2D_Trace_M(c_Linked_List_Handler* p_LL, c_Linked_List_Handler* p_LL_X, c_Linked_List_Handler* p_LL_Y, uint64_t p_X = 0, uint64_t p_Y = 0)
    {
        //---std::cout << "\n bp_2D_Trace_M <=- " << NID << " " << p_X << " " << p_Y;

        //If a left leg exists then initiate a backpropagation along it, then along the right side.
        if (Dendrite_Count != 0)
        {
            //---std::cout << "*";
            if (Dendrites[1] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[1]->Type != 2) && (Dendrites[1]->Type != 3))
                {
                    Dendrites[1]->bp_2D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_X, p_Y + 1);
                }
            }

            //---std::cout << "*";
            if (Dendrites[2] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[2]->Type != 2) && (Dendrites[2]->Type != 3))
                {
                    Dendrites[2]->bp_2D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_X + 1, p_Y);
                }
            }
            //---std::cout << "*";
            if (Dendrites[3] != NULL)
            {
                //If not a treetop then call _F
                if ((Dendrites[3]->Type != 2) && (Dendrites[3]->Type != 3))
                {
                    Dendrites[3]->bp_2D_Trace_M(p_LL, p_LL_X, p_LL_Y, p_X + 1, p_Y + 1);
                }
            }
        }
        else
        {
            //Output the state
            //---std::cout << " <" << NID << " :: " << State << "> ";
            p_LL->new_LL(State);
            p_LL_X->new_LL(p_X);
            p_LL_Y->new_LL(p_Y);
        }
    }


    void output_Node_Raw()
    {
        std::cout << "\n Node_ID (NID): " << NID << " ";
        std::cout << " RC: " << RC;
        std::cout << " Type: " << Type;
        std::cout << " State { " << State << " }";
        std::cout << " --- Dendrites: ";
        for (int cou_D = 0; cou_D < Dendrite_Count; cou_D++)
        {
            std::cout << " [" << cou_D << "] " << Dendrites[cou_D]->NID << " Weight: " << Dendrite_Weights[cou_D] << " | ";
        }
        std::cout << " --- Axon_Hillock_Count: " << Axon_Hillock_Count;
        for (int cou_H = 0; cou_H < Axon_Hillock_Count; cou_H++)
        {
            std::cout << " Axons[" << cou_H << "] <" << Axon_Count[cou_H] << ">: ";
            for (int cou_A = 0; cou_A < Axon_Count[cou_H]; cou_A++)
            {
                std::cout << " [" << cou_A << "] " << Axons[cou_H][cou_A]->NID;
            }
        }
    }

    void output_Node_Char()
    {
        std::cout << char(this);
    }
};