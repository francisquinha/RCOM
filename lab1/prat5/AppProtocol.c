//faltam includes

//================================================================================================================
//AUX FUNCS
//================================================================================================================

/*ESTOU A PRESSUPOR QUE O ARRAY DATA-BUFFER E DINAMICO (USA ALLOCS)*/
int update_received_data(char* data_buffer, int data_buffer_length, char* newdata_buffer, int newdata_buffer_length)
{
	if (data_buffer_length > 0)
	{
		if ((data_buffer = (char*)malloc(newdata_buffer_length)) == NULL)
		{
			perror("update_received_data:");
			return -1;
		}
		data_buffer_length = newdata_buffer_length;
		memmov(data_buffer, newdata_buffer, newdata_buffer_length);
	}
	else
	{
		data_buffer = (char*)realloc(data_buffer, data_buffer_length + newdata_buffer_length);
		memmov(data_buffer + data_buffer_length, newdata_buffer, newdata_buffer_length);
		data_buffer_length += newdata_buffer_length;
	}
	return OK;
}

//================================================================================================================
//MAIN FUNCS
//================================================================================================================

int sendControlPacket()
{
	return OK;
}

int sendInfoPacket()
{
	return OK;
}

int receiveControlPacket()
{
	return OK;
}

int receiveInfoPacket()
{
	return OK;
}