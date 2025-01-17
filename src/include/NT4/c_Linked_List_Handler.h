
//This is the linked list for the buffer.
class c_Linked_List
{
public:

    //The next node in the chain.
    c_Linked_List* Next;
    c_Linked_List* Previous;
    uint64_t Quanta;

    c_Linked_List()
    {
        Next = NULL;
        Previous = NULL;
        Quanta = 0;
    }

    ~c_Linked_List()
    {
        Quanta = 0;
        //delete Next; Handled by the handler.
    }

    //Outputs the linked list.
    //-- !WARNING! RECURSIVE FUNCTION ON LINKED LIST! CALLING THIS WILL CAUSE STACK CORRUPTION ERROR WITH ENOUGH NODES!
    void output_LL()
    {
        //*std::cout << " [";
        std::cout << " $" << Quanta << "]  ";
        if (Next != NULL) { Next->output_LL(); }
    }
};

//This handles the linked list creation and deletion so that recursive destruction does not crash the stack.
class c_Linked_List_Handler
{
public:

    //The nodes in the linked list.
    c_Linked_List* Root;
    c_Linked_List** Current_LL;
    int Depth;

    c_Linked_List_Handler()
    {
        Root = NULL;
        Current_LL = &Root;
        Depth = 0;
    }

    ~c_Linked_List_Handler()
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

    //Adds a node with the quanta of data passed.
    c_Linked_List* new_LL(uint64_t p_Quanta)
    {
        c_Linked_List* tmp_LL = NULL;

        *Current_LL = new c_Linked_List;

        (*Current_LL)->Quanta = p_Quanta;

        tmp_LL = *Current_LL;

        (*Current_LL)->Previous = tmp_LL;

        Current_LL = &(*Current_LL)->Next;

        Depth++;

        return tmp_LL;
    }

    //Resets the linked list.
    void reset()
    {
        c_Linked_List* tmp_LL = Root;
        Root = NULL;
        c_Linked_List* tmp_LL_Next = NULL;

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
        c_Linked_List* tmp_LL = Root;

        c_Linked_List* tmp_LL_Next = NULL;

        std::cout << "\n Output Linked List:";

        std::cout << " Depth: " << Depth;
        while (tmp_LL != NULL)
        {
            tmp_LL_Next = tmp_LL->Next;

            //Hmm, might change the tmp_LL->NID to a different name so it isn't NID->NID
            std::cout << "\n   -";
            
            std::cout << tmp_LL->Quanta;

            std::cout << "\n";


            tmp_LL = tmp_LL_Next;
        }
    }
};