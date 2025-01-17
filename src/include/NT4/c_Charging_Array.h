
//These classes handle the charging buffers and charging array.



//This is the linked list for the buffer.
class c_Charging_Linked_List
{
public:

    //The next node in the chain.
    c_Charging_Linked_List* Next;
    c_Charging_Linked_List* Previous;
    double Charge;
    c_Node* NID;

    c_Charging_Linked_List()
    {
        Next = NULL;
        Previous = NULL;
        Charge = 0;
        NID = NULL;
    }

    ~c_Charging_Linked_List()
    {
        Charge = 0;
        NID = NULL;
        //delete Next; Handled by the handler.
    }

    //Outputs the linked list.
    //-- !WARNING! RECURSIVE FUNCTION ON LINKED LIST! CALLING THIS WILL CAUSE STACK CORRUPTION ERROR WITH ENOUGH NODES!
    void output_LL()
    {
        std::cout << "\n [";
        if (NID != NULL) { NID->bp_O(1); }
        std::cout << " $" << Charge << "]  ";
        if (Next != NULL) { Next->output_LL(); }
    }
};

//This handles the linked list creation and deletion so that recursive destruction does not crash the stack.
class c_Charging_Linked_List_Handler
{
public:

    //The nodes in the linked list.
    c_Charging_Linked_List* Root;
    c_Charging_Linked_List** Current_LL;
    int Depth;

    c_Charging_Linked_List_Handler()
    {
        Root = NULL;
        Current_LL = &Root;
        Depth = 0;
    }

    ~c_Charging_Linked_List_Handler()
    {
        reset();
    }

    //Init
    void init()
    {
        Root = NULL;
        Current_LL = &Root;
        Depth = 0;
    }

    //Adds a node.
    c_Charging_Linked_List* new_LL(c_Node* p_NID, double p_Charge)
    {
        c_Charging_Linked_List* tmp_LL = NULL;

        *Current_LL = new c_Charging_Linked_List;

        (*Current_LL)->Charge = p_Charge;
        (*Current_LL)->NID = p_NID;

        tmp_LL = *Current_LL;

        (*Current_LL)->Previous = tmp_LL;

        Current_LL = &(*Current_LL)->Next;

        Depth++;

        return tmp_LL;
    }

    //Resets the linked list.
    void reset()
    {
        c_Charging_Linked_List* tmp_LL = Root;
        Root = NULL;
        c_Charging_Linked_List* tmp_LL_Next = NULL;

        while (tmp_LL != NULL)
        {
            tmp_LL_Next = tmp_LL->Next;
            tmp_LL->Next = NULL;
            tmp_LL->Previous = NULL;
            delete tmp_LL;
            tmp_LL = NULL;
            tmp_LL = tmp_LL_Next;
        }

        if (Root != NULL) { delete Root; }
        Root = NULL;
        Current_LL = &Root;

        Depth = 0;
    }

    //Outputs the linked list.
    void output()
    {
        c_Charging_Linked_List* tmp_LL = Root;
        
        c_Charging_Linked_List* tmp_LL_Next = NULL;

        std::cout << "\n ----Charging_Linked_List:";
        while (tmp_LL != NULL)
        {
            tmp_LL_Next = tmp_LL->Next;

            //Hmm, might change the tmp_LL->NID to a different name so it isn't NID->NID
            std::cout << "\n   -";
            
            std::cout << tmp_LL->NID->NID;

            std::cout << "\n";

            tmp_LL->NID->bp_O();

            tmp_LL = tmp_LL_Next;
        }
    }
};


class c_Charging_Buffer_Node
{
    friend class c_Charging_Buffer_Tree;
private:

    c_Charging_Buffer_Node* Right;
    c_Charging_Buffer_Node* Center;
    c_Charging_Buffer_Node* Left;

public:

    uint64_t Data;

    c_Charging_Linked_List* LL; 

    c_Charging_Buffer_Node()
    {
        Right = NULL;
        Center = NULL;
        Left = NULL;
        LL = NULL;
        Data = 0;
    }

    ~c_Charging_Buffer_Node()
    {
        LL = NULL;
        delete Right; Right = NULL;
        delete Center; Center = NULL;
        delete Left; Left = NULL;
    }

};

class c_Charging_Buffer_Tree
{
private:


public:
    c_Charging_Buffer_Node* Root;
    c_Charging_Buffer_Node** Current;//the current node that will allow referencing outside of the search function
    bool flg_Foundit;//the flag that set when a node is found already set

    //For handling the current search.
    uint64_t tmp_Data;
    uint64_t tmp_SData;

    c_Charging_Buffer_Tree()
    {
        Root = NULL;
        Current = NULL;
        flg_Foundit = false;

        tmp_Data = 0;
        tmp_SData = 0;
    }

    ~c_Charging_Buffer_Tree()
    {
        delete Root;
    }

    void init()
    {
        Root = NULL;
        Current = NULL;
        flg_Foundit = false;
    }

    //Now returning whether or not the node was found as the foundit flag does not work with multiple threads.
    //This returns the node at the end of the chain, whether it was found or not does not matter the 
    //buffer node is returned and has to be checked for values.
    c_Charging_Buffer_Node* search(c_Node* p_Data)
    {
        flg_Foundit = false;

        if (p_Data == NULL) { Current = NULL; return NULL; }

        uint64_t tmp_SSData;
        tmp_SSData = uint64_t(p_Data);



        //--std::cout << "\n\n SEARCHING ->" << tmp_SData.U;
        //system("PAUSE > NULL");
        return query_Node(Root, tmp_SSData, 31);
    }

    c_Charging_Buffer_Node* query_Node(c_Charging_Buffer_Node*& p_Node, uint64_t p_Data, int p_Shift)
    {
        //These are used because we have to be carefull not to leave Mutex locked with a return.
        c_Charging_Buffer_Node* tmp_Return = NULL;
        bool flg_Done = false;

        uint64_t tmp_DData;
        tmp_DData = p_Data;
        tmp_DData = tmp_DData >> p_Shift;
        //--std::cout << "\n\n\n tmp_DData.U = " << tmp_DData.U << " p_Shift = " << p_Shift;


        //--std::cout << "\n\n\t __QN__ tmp_DData.U->" << tmp_DData.U << " p_Data.U->" << p_Data.U;

        if (p_Node == NULL)
        {
            //--std::cout << "\n\t   Node is NULL";
            p_Node = new c_Charging_Buffer_Node;
            p_Node->Right = NULL;
            p_Node->Center = NULL;
            p_Node->Left = NULL;
            if (p_Shift > 1)
            {
                //--std::cout << "\n\t   tmp_DData.U > 1";
                p_Node->Data = tmp_DData;
            }
            else
            {
                //--std::cout << "\n\t   tmp_DData.U !> 1";
                p_Node->Data = p_Data;
                Current = &p_Node;
                flg_Foundit = false;
                tmp_Return = p_Node;
                flg_Done = true;
                //return 0;
            }
        }

        //If the node was not found then return NULL;
        if (flg_Done) { return tmp_Return; }

        //If the current data matches the tmp data then another depth is explored.
        if (p_Node->Data == tmp_DData && p_Shift > 0)
        {
            //--std::cout << "\n\t   p_Node->Data.U (" << p_Node->Data.U << ") == tmp_DData.U (" << tmp_DData.U << ")";

            //--std::cout << "\t tmp_DData.U = " << tmp_DData.U;

            return query_Node(p_Node->Center, p_Data, (p_Shift - 1));
        }

        //If the node data matches the given data exactly the node has been found.
        //This is not locked because even if the Current & flg_Foundit are overwritten they will not be
        //used by the threads, but will work with the standard single thread.
        if (p_Node->Data == p_Data)
        {
            //--std::cout << "\n\t    p_Node->Data.NR(" << p_Node->Data.NR << ") == p_Data.NR(" << p_Data.NR << ")";
            Current = &p_Node;
            flg_Foundit = true;
            tmp_Return = p_Node;
            flg_Done = true;
            //return 1;
        }

        //If the node was found then return tmp_Return;
        if (flg_Done) { return tmp_Return; }

        if (tmp_DData < p_Node->Data)
        {
            //--std::cout << "\n\t    tmp_DData.U (" << tmp_DData.U << ") < p_Node->Data.U(" << p_Node->Data.U << ")";
            return query_Node(p_Node->Left, p_Data, p_Shift);
        }
        if (tmp_DData > p_Node->Data)
        {
            //--std::cout << "\n\t    tmp_DData.U (" << tmp_DData.U << ") > p_Node->Data.U(" << p_Node->Data.U << ")";
            return query_Node(p_Node->Right, p_Data, p_Shift);
        }
        return tmp_Return;
    }

    void output_Tree()
    {
        output_Node(Root, 0);
    }

    void output_Node(c_Charging_Buffer_Node*& p_Node, int p_Tab)
    {
        if (p_Node == NULL) { return; }
        output_Node(p_Node->Left, (p_Tab));
        //---std::cout << "\n";
        for (int cou_Index = 0; cou_Index < p_Tab; cou_Index++)
        {
            //---std::cout << "  ";
        }
        //---std::cout << p_Node->Data;
        if (p_Node->LL != NULL) { std::cout << "->" << p_Node->LL->NID->NID; }
        output_Node(p_Node->Center, (p_Tab + 1));
        output_Node(p_Node->Right, (p_Tab));
    }

    c_Charging_Linked_List* get_Node_LL(c_Charging_Buffer_Node*& p_Node)
    {
        if (p_Node != NULL) {
            return p_Node->LL;
        }
        else {
            return NULL;
        }
    }

    void set_Node_LL(c_Charging_Buffer_Node*& p_Node, c_Charging_Linked_List* p_LL)
    {
        if (p_Node != NULL)
        {
            p_Node->LL = p_LL;
        }
    }

    c_Charging_Linked_List* set_Current_Node_LL(c_Charging_Linked_List* p_LL)
    {
        set_Node_LL(*Current, p_LL);
        return p_LL;
    }

    c_Charging_Linked_List* get_Current_Node_LL()
    {
        return get_Node_LL(*Current);
    }


    //Resets the tree.
    void reset()
    {
        delete Root;
        Root = NULL;

        Current = NULL;
        flg_Foundit = false;
    }

};



//The class that houses the charging buffer tree and the LL.
class c_Charging_Buffer
{
private:

public:

    //The search tree.
    c_Charging_Buffer_Tree Tree;

    //The linked lists.
    c_Charging_Linked_List_Handler Current_Charge;

    c_Charging_Linked_List_Handler Output;

    c_Charging_Linked_List_Handler Treetops;

    //The treetop tree. What a stupid name.
    c_Charging_Buffer_Tree Treetop_Tree;

    //Flags
    bool flg_Not_Done;

    int Input_Position;

    //Settings & trackers for the settings.
    double Base_Charge;
    double Current_Highest_Charge;
    double Previous_Highest_Charge;
    double Treetops_Highest_Charge;

    double Modifier_Charge;
    double Action_Potential_Threshold;
    double RC_Limiter;

    c_Charging_Buffer()
    {
        flg_Not_Done = 0;
        Input_Position = 0;

        Current_Highest_Charge = 0;
        Treetops_Highest_Charge = 0.0;
        Base_Charge = 10.00;
        Previous_Highest_Charge = Base_Charge;
        Modifier_Charge = 1.0f;
        Action_Potential_Threshold = 0.0f;
        RC_Limiter = Base_Charge;
    }

    ~c_Charging_Buffer()
    {
        Current_Charge.reset();
        Output.reset();
        Treetops.reset();
        Treetop_Tree.reset();
    }

    void reset()
    {
        Current_Charge.reset();
        Output.reset();

        //reset_Treetops();

        flg_Not_Done = 0;
        Input_Position = 0;
    }

    void reset_Treetops()
    {
        Treetops.reset();
        Treetop_Tree.reset();
        Treetops_Highest_Charge = 0.0;
    }

    //Hyperparameters
    void set_Base_Charge(double p_Base_Charge)
    {
        Base_Charge = p_Base_Charge;
    }

    void set_Modifier_Charge(double p_Modifier_Charge)
    {
        Modifier_Charge = p_Modifier_Charge;
    }

    void set_Action_Potential_Threshold(double p_Action_Potential_Threshold)
    {
        Action_Potential_Threshold = p_Action_Potential_Threshold;
    }

    void set_RC_Limiter(double p_RC_Limiter)
    {
        RC_Limiter = p_RC_Limiter;
    }

    double get_Base_Charge()
    {
        return Base_Charge;
    }

    double get_Modifier_Charge()
    {
        return Modifier_Charge;
    }

    double get_Action_Potential_Threshold()
    {
        return Action_Potential_Threshold;
    }

    double get_RC_Limiter()
    {
        return RC_Limiter;
    }

    double get_Treetops_Highest_Charge()
    {
        return Treetops_Highest_Charge;
    }

    //=====--            --=====//
    //==--     CHARGING     --==//
    //=====--            --=====//


    //--  CHARGING FOR ORIGINAL ALGO, USED FOR MSC WHERE ALL PATTERNS ARE TO BE FOUND

    //Charges the outputs back into the charging buffer.
    // //Reinstated the leg charging at this level, this is for charging nodes based on their position, so a state tier node at position 2 charges only axon hillock 2
    // 
    // -The Following doesn't apply as the method was moved to CAN level leg firing order arrays. Left these comments as the CAN method limits each node to one charging type per charge so it may be reinstated in the future.
    // --By keeping the leg charging localized to the parameter we can allow different charging methods to be used on the same node during the same run. I don't know why yet, but this preserves that functionality.
    // --The leg charging determines the hillock to charge by iterating through it.
    //void charge_Outputs(int * p_Flg_Leg_Charging, int p_Flg_Leg_Charging_Count)
    void charge_Outputs()
    {
        c_Charging_Linked_List* tmp_LL = Output.Root;

        Previous_Highest_Charge = Current_Highest_Charge;
        Current_Highest_Charge = 0;

        double tmp_Charge = 0;
        double tmp_Charge_Percentage = 0.0f;
        float tmp_RC = 0.0f;

        while (tmp_LL != NULL)
        {
            tmp_Charge = 0.0;
            tmp_Charge_Percentage = 0.0;

            if (tmp_LL->NID == NULL) { tmp_LL = tmp_LL->Next; continue; }

            //---std::cout << "\n tmp_Charge_Percentage " << tmp_Charge_Percentage << " = ((tmp_LL->Charge " << tmp_LL->Charge << "  * Modifier_Charge " << Modifier_Charge << " ) / Previous_Highest_Charge " << Previous_Highest_Charge << " );";
            /*
            if (tmp_RC > RC_Limiter) { tmp_RC = RC_Limiter; }
            tmp_RC = tmp_LL->NID->RC * (1 - (tmp_LL->NID->RC / RC_Limiter));
            if (tmp_RC < 0) { tmp_RC = 0; }

            tmp_Charge_Percentage = (((tmp_LL->Charge + tmp_RC) * Modifier_Charge) / Previous_Highest_Charge);
            */
            tmp_Charge_Percentage = (((tmp_LL->Charge) * Modifier_Charge) / Previous_Highest_Charge);
            
            //---std::cout << "\n --- tmp_Charge_Percentage " << tmp_Charge_Percentage << " = ((tmp_LL->Charge " << tmp_LL->Charge << "  * Modifier_Charge " << Modifier_Charge << " ) / Previous_Highest_Charge " << Previous_Highest_Charge << " );";
            tmp_Charge = double(tmp_Charge_Percentage * Base_Charge);

            tmp_LL->NID->Charge = tmp_Charge;

            if (tmp_Charge < (Base_Charge * Action_Potential_Threshold)) { tmp_LL = tmp_LL->Next; continue; }

            //We use leg index selection instead of L/R
            for (int cou_H = 0; cou_H < tmp_LL->NID->Axon_Hillock_Count; cou_H++)
            {
                for (int cou_A = 0; cou_A < tmp_LL->NID->Axon_Count[cou_H]; cou_A++)
                {
                    //submit(tmp_LL->NID->Axons[cou_H][cou_A], tmp_Charge);

                    //---std::cout << "\n NID " << tmp_LL->NID->NID << " Axons[ " << cou_H << " ][ " << cou_A << " ]->Dendrite_Weights[ " << cou_A << " ] " << tmp_LL->NID->Axons[cou_H][cou_A]->Dendrite_Weights[cou_H];

                    //This is for the double legged node charging, may separate out in the future as not all nodes need to do this.
                    submit(tmp_LL->NID->Axons[cou_H][cou_A], (tmp_Charge * tmp_LL->NID->Axons[cou_H][cou_A]->Dendrite_Weights[cou_H]));
                }
            }

            if (tmp_LL->NID->Axon_Hillock_Count == 0)
            {
                submit_Treetop((tmp_LL->NID), (tmp_Charge));

            }

            tmp_LL = tmp_LL->Next;
        }
    }

    //=====--                   --=====//
    //==--     SUBMISSION NODE     --==//
    //=====--                   --=====//

    //Accepts a submission of nodes to charge, charges the given leg. Used mainly for Chrono.
    //Use this then gather and begin charging loop.
    void charge_Given_Leg(c_Node* p_Node, double p_Charge = 10.00, int p_Leg = 0)
    {
        p_Node->Charge = p_Charge;

        //Search for the submitted node, if found add the charge.
        if (p_Leg < p_Node->Axon_Hillock_Count)
        {
            //This shouldn't run if the axon count is 0, meaning if the Axons[p_Leg] == NULL it will be fine.
            for (int cou_A = 0; cou_A < p_Node->Axon_Count[p_Leg]; cou_A++)
            {
                submit(p_Node->Axons[p_Leg][cou_A], p_Charge);
                //---std::cout << "\n ~[" << p_Leg << "][" << cou_A << "]~  A->" << p_Node->Axons[p_Leg][cou_A];
                //---p_Node->Axons[p_Leg][cou_A]->bp_O();
            }
        }
        else
        {
            //---std::cerr << "\n \\(o.O)--  Error: p_Leg (" << p_Leg << ") Exceeds p_Node::Axon_Hillock_Count...";
            //---p_Node->output_Node_Raw();
            //---std::cout << " Pattern: ";
            //---p_Node->bp_O();
        }
    }

    //Accepts a submission of nodes to charge, charges the given legs. Used mainly for Chrono.
    //Use this then gather and begin charging loop.
    //It is assumed p_Leg is the same depth as the input.
    void charge_Given_Legs(c_Node* p_Node, int p_Leg_Count, int* p_Leg, double p_Charge = 10.00)
    {
        if (p_Node != NULL)
        {
            int tmp_Top = p_Node->Axon_Hillock_Count;
            if (tmp_Top > p_Leg_Count) { tmp_Top = p_Leg_Count; }

            for (int cou_H = 0; cou_H < tmp_Top; cou_H++)
            {
                if (p_Leg[cou_H])
                {
                    //This shouldn't run if the axon count is 0, meaning if the Axons[p_Leg] == NULL it will be fine.
                    for (int cou_A = 0; cou_A < p_Node->Axon_Count[cou_H]; cou_A++)
                    {
                        submit(p_Node->Axons[cou_H][cou_A], p_Charge);
                        //---std::cout << "\n ~[" << cou_H << "][" << cou_A << "]~  A->" << p_Node->Axons[cou_H][cou_A];
                        //---p_Node->Axons[cou_H][cou_A]->bp_O();
                    }
                }
            }
        }
    }

    //=====--              --=====//
    //==--     SUBMISSION     --==//
    //=====--              --=====//

    //Accepts a submission of nodes to charge, used for CAN.
    void submit(c_Node* p_Node, double p_Charge = 10.00)
    {
        if (p_Node == NULL) { std::cerr << "\n (o.O) Error! p_Node is NULL to submit!"; }

        //Search for the submitted node, if found add the charge.
        Tree.search(p_Node);

        if (Tree.flg_Foundit)
        {
            (Tree.get_Current_Node_LL())->Charge += p_Charge;
            if ((Tree.get_Current_Node_LL())->Charge > Current_Highest_Charge) { Current_Highest_Charge = (Tree.get_Current_Node_LL())->Charge; }
        }

        if (!Tree.flg_Foundit && p_Node != NULL)
        {
            if (p_Charge > Current_Highest_Charge) { Current_Highest_Charge = p_Charge; }

            Tree.set_Current_Node_LL(Current_Charge.new_LL(p_Node, p_Charge));
        }
    }
    
    void submit_Treetop(c_Node* p_Node, double p_Charge)
    {
        if (p_Node == NULL) { std::cerr << "\n (O.O) Error! p_Node is NULL to submit_Treetop!"; }

        //---std::cout << "\n [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]  ~~~ submit_Treetop " << p_Node << " p_Charge " << p_Charge;

        //Search for the submitted node, if found add the charge.
        Treetop_Tree.search(p_Node);

        if (Treetop_Tree.flg_Foundit)
        {
            (Treetop_Tree.get_Current_Node_LL())->Charge += p_Charge;
        }

        if (!Treetop_Tree.flg_Foundit && p_Node != NULL)
        {
            Treetop_Tree.set_Current_Node_LL(Treetops.new_LL(p_Node, p_Charge));
        }

        if ((Treetop_Tree.get_Current_Node_LL())->Charge > Treetops_Highest_Charge)
        { 
            Treetops_Highest_Charge = (Treetop_Tree.get_Current_Node_LL())->Charge; 
        }

        //---std::cout << "   ~~~ flg_foundit " << Treetop_Tree.flg_Foundit << " Treetops.charge " << (Treetop_Tree.get_Current_Node_LL())->Charge;
    }

    //=====--             --=====//
    //==--     GATHERING     --==//
    //=====--             --=====//

    //For gathering the Opp
    //>Target Tier - 1
    //>charge
    //>gather
    //>flush(Target_Tier)

    //Gathers the output nodes and resets the tree for the next iteration.
    void gather()
    {
        //Delete the old Output.
        Output.reset();

        //Reassign the linked lists.
        Output.Root = Current_Charge.Root;
        Output.Depth = Current_Charge.Depth;

        Current_Charge.Root = NULL;
        Current_Charge.reset();

        //Delete the tree.
        Tree.reset();

        //Sets the done flag if Output == NULL.
        flg_Not_Done = 1;
        if ((Output.Root == NULL)) { flg_Not_Done = 0; }
    }

    //Charges the outputs back into the charging buffer.
    double gather_Treetopss()
    {
        std::cout << "\n\n\n\n\n\n gather_Treetops()";
        c_Charging_Linked_List* tmp_LL = Treetops.Root;
        c_Charging_Linked_List_Handler tmp_Treetops_LLH;;
        tmp_Treetops_LLH.Root = Treetops.Root;

        double tmp_Current_Highest_Charge = 0;

        double tmp_Charge = 0;

        //Find the current highest charge.
        while (tmp_LL != NULL)
        {
            if (tmp_LL->NID == NULL) { tmp_LL = tmp_LL->Next; continue; }

            if (tmp_Current_Highest_Charge < tmp_LL->Charge) { tmp_Current_Highest_Charge = tmp_LL->Charge; }

            tmp_LL = tmp_LL->Next;
        }

        std::cout << "\n Treetop_Count: " << Treetops.Depth;

        Treetops.Root = NULL;
        tmp_LL = tmp_Treetops_LLH.Root;

        while (tmp_LL != NULL)
        {
            if (tmp_LL->NID == NULL) { tmp_LL = tmp_LL->Next; continue; }

            std::cout << "\n  ------------ [" << tmp_LL->NID->NID << "] " << tmp_LL->Charge;

            tmp_Charge = (((tmp_LL->Charge * Modifier_Charge) / tmp_Current_Highest_Charge) * Base_Charge);

            std::cout << " ------- " << tmp_Charge;
            std::cout << " < " << (Base_Charge * Action_Potential_Threshold);
            if (tmp_Charge < (Base_Charge * Action_Potential_Threshold)) { std::cout << " XXXXXX "; tmp_LL = tmp_LL->Next; continue; }


            Treetops.new_LL(tmp_LL->NID, (tmp_Charge));

            tmp_LL = tmp_LL->Next;
        }

        tmp_Treetops_LLH.reset();
        Treetop_Tree.reset();
        Treetops.reset();

        return tmp_Current_Highest_Charge;
    }

    //=====--                       --=====//
    //==--     CLEANUP & ALLOCATION    --==//
    //=====--                       --=====//


    //Destroys the output buffers.
    void destroy_Output_Buffers()
    {
        //Delete the old Output.
        Output.reset();
    }

    //=====--          --=====//
    //==--     OUTPUT     --==//
    //=====--          --=====//

    //Outputs all of the buffers.
    void output_All_Buffers()
    {
        //system("CLS");
        std::cout << "\n Charge_LL->" << Current_Charge.Depth << " ->";
        output_Charge_LL();
        std::cout << "\n Output_LL->" << Output.Depth << " ->";
        output_Output_LL();
        std::cout << "\n Treetops->" << Treetops.Depth << " ->";
        output_Treetops();
        std::cout << "\n Treetop_Tree: \n";
        output_Treetop_Tree();
    }

    //Outputs the buffer.
    void output_Tree()
    {
        Tree.output_Tree();
    }

    //Outputs the buffer.
    void output_Treetop_Tree()
    {
        Treetop_Tree.output_Tree();
    }

    //Outputs the Current_Charge_LL.
    void output_Charge_LL()
    {
        //---std::cout << "\n\t Current_Charge_LL->";
        if (Current_Charge.Root == NULL) { std::cout << "empty..."; return; }
        Current_Charge.Root->output_LL();
        //---std::cout << " ~Fing"; std::cout.flush();
    }

    //Outputs the Output_LL.
    void output_Output_LL()
    {
        //---std::cout << "\n\t Output_LL->";
        if (Output.Root == NULL) { std::cout << "empty..."; return; }
        Output.Root->output_LL();
        //---std::cout << " ~Fing"; std::cout.flush();
    }

    //Outputs the Treetops.
    void output_Treetops()
    {
        std::cout << "\n\t Treetops_LL->";
        if (Treetops.Root == NULL) { std::cout << "empty..."; return; }
        //Treetops.Root->output_LL();
        Treetops.output();
        //---std::cout << " ~Fing"; std::cout.flush();
    }
};


