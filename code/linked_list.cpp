struct node
{
	void *data;
	node *next;
	node *prev;
};

typedef node* linked_list;

void list_insert_end(node **list, void *data, size_t size)
{
	node *tmp = (node *)malloc(sizeof(node));
	tmp->data = malloc(size);

	for(int i=0;i<size;i++)
	{
		*((char*)tmp->data+i) = *((char*)data+i);
	}

	tmp->prev = tmp->next = NULL;

	if(!(*list))
	{
		*list = tmp;
	}
	else
	{
		node *n = *list;
		node *p = NULL;

		while(n)
		{
			p = n;
			n = n->next;
		}

		p->next = tmp;
	}
}

void list_insert_beg(node **list, void *data, size_t size)
{
	node *tmp = (node *)malloc(sizeof(node));
	tmp->data = malloc(size);

	for(int i=0;i<size;i++)
	{
		*((char*)tmp->data+i) = *((char*)data+i);
	}

	tmp->prev = tmp->next = NULL;

	if(!(*list))
	{
		*list = tmp;
	}
	else
	{
		tmp->next = *list;
		*list = tmp;
	}
}


void list_delete(node **list, node *node)
{
	if(!node->prev && !node->next)
	{
		*list = 0;
	}
	else if(!node->prev)
	{
		node->next->prev = NULL;
		*list = node->next;
	}
	else if(!node->next)
	{
		node->prev->next = NULL;
	}
	else
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	free(node->data);
	free(node);
}

node * list_search(node **list, void *data, size_t size)
{
	node *node = *list;
	while(node)
	{
		int is_equal = 1;

		for(int i=0;i<size;i++)
		{
			if(*((char*)node->data+i) != *((char*)data+i))
			{
				is_equal = 0;
			}
		}

		if(is_equal)
		{
			break;
		}

		node = node->next;
	}
	return node;
}
