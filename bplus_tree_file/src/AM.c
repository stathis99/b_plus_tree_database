#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "AM.h"


#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return AME_ERROR;        \
  }                         \
}

int AM_errno = AME_OK;

void AM_Init() 
{
	BF_Init(LRU);
	for(int i=0;i<20;i++)
	{
		AM_Array[i].free_place = 0;
		Scan_Array[i].block_num = 0;
		for(int j=0;j<20;j++)
		{
			AM_Array[i].file_name[j] = '0';
		}
	}
	return;
}


int AM_CreateIndex(char *fileName, char attrType1, int attrLength1, char attrType2, int attrLength2) 
{
	BF_Block *block;
	int fd;
	char* data;
	BF_Block_Init(&block);
	CALL_BF(BF_CreateFile(fileName));
	BF_OpenFile(fileName,&fd);
	CALL_BF(BF_AllocateBlock(fd, block));
	data = BF_Block_GetData(block);
	memcpy(data,&attrType1,sizeof(char));
	memcpy(data+sizeof(char),&attrType2,sizeof(char));
	memcpy(data+(2*sizeof(char)),&attrLength1,sizeof(int));
	memcpy(data+(2*sizeof(char)+sizeof(int)),&attrLength2,sizeof(int));
	BF_Block_SetDirty(block);
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);
	CALL_BF(BF_CloseFile(fd));
	return AME_OK;
}


int AM_DestroyIndex(char *fileName) {
	
	return AME_OK;
}


int AM_OpenIndex (char *fileName) 
{
	BF_Block *block;
	char* data;
	char check[2];
	int fd;
	BF_Block_Init(&block);
	CALL_BF(BF_OpenFile(fileName,&fd));
	CALL_BF(BF_GetBlock(fd, 0 , block));
	data = BF_Block_GetData(block);
	memcpy(check,data,2*sizeof(char));
//	if(strcmp(check,"cc")==0 || strcmp(check,"ci")==0 || strcmp(check,"cf")==0 || strcmp(check,"ic")==0 || strcmp(check,"ii")==0 || strcmp(check,"if")==0 || strcmp(check,"fc")==0 || strcmp(check,"fi")==0 || strcmp(check,"ff")==0)
//	{
		for(int i=0;i<20;i++)
		{
			if(AM_Array[i].free_place == 0)
			{
				strcpy(AM_Array[i].file_name,fileName);
				AM_Array[i].file_desc = fd;
				AM_Array[i].free_place = 1;
				AM_errno = i;
				break;
			}
		}
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		printf("%d\n",AM_errno);
		return AM_errno;
//	}
//	else
///	{
//		CALL_BF(BF_UnpinBlock(block));
//		BF_Block_Destroy(&block);
//		AM_errno = -2;
//		return AM_errno;	
//	}	
}


int AM_CloseIndex (int fileDesc) 
{
	
	return AME_OK;
}

char* init_tree(int fd,void* key,int length_key,int key_type){
		BF_Block *block;
	  char* data;
		BF_Block_Init(&block);
		BF_AllocateBlock(fd, block);
		data = BF_Block_GetData(block);
	  int counter = 1;
		int left_addr = 3; //to aristero leaf
		int right_addr = 2;


		data[0] = 'I';
		memcpy(data+1,&counter,sizeof(int));
		memcpy(data+1+sizeof(int),&left_addr,sizeof(int));
		memcpy(data+1+2*sizeof(int),key,length_key);
		memcpy(data+1+2*sizeof(int)+length_key,&right_addr,sizeof(int));

		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);
		return data;

}


int scan_index(char* my_block,void* key,int length,char* type){
	int counter;
	memcpy(&counter,my_block+1,sizeof(int));
	my_block = my_block+1+2*sizeof(int);//my block points to the first key
	if(type == 'c'){

		char* mykey;
		memcpy(mykey,key,length);


		for(int i=0;i<counter;i++){
			if(strcmp(mykey,my_block)<0){
				return *(my_block-sizeof(int));
			}
			my_block = my_block + length + sizeof(int);
		}
		return *(my_block-sizeof(int));
	}else if(type == 'f'){
		float mykey;
		memcpy(&mykey,key,length);


		for(int i=0;i<counter;i++){
			if(mykey < *my_block){
				return *(my_block-sizeof(int));
			}
			my_block = my_block + length + sizeof(int);
		}
		return *(my_block-sizeof(int));
	}else if(type == 'i'){
		int mykey;
		memcpy(&mykey,key,length);


		for(int i=0;i<counter;i++){
			if(mykey < *my_block){
				return *(my_block-sizeof(int));
			}
			my_block = my_block + length + sizeof(int);
		}
		return *(my_block-sizeof(int));
	}

}

/*afti h synarthsh dexetai ena block kai topothetei ena entry mesa sto block
an xwraei epistrefei ena an oxi 0
*/
int leaf_insert(char* my_block,void* value1,void* value2,char* type1,int length1,int length2){
	int limit = (BF_BLOCK_SIZE - 2*sizeof(int) - sizeof(char))/(length1+length2);

	int counter;
	memcpy(&counter,my_block+1,sizeof(int));
 	if(counter >= limit){
		return 0;
	}else{

		int temp_counter = counter + 1;
		memcpy(my_block+1,&temp_counter,sizeof(int));//auxanei ton counter

		my_block = my_block + 1 + 2*sizeof(int) ;

		 if(type1 == 'c'){
		 char* v1 = value1;

			 for(int i=0;i<counter;i++){
				 if(strcmp(v1,my_block)<0){
					memcpy(my_block+length1+length2,my_block,(counter-i)*(length1+length2));
				 	memcpy(my_block,v1,length1);
				 	memcpy(my_block+length1,value2,length2);
				 	return 1;
				 }

				 my_block = my_block + length1 + length2;

			 }

			 memcpy(my_block,v1,length1);
			 memcpy(my_block+length1,value2,length2);
			return 1;
		 }else if(type1 == 'f'){
			 float* v1 = value1;
			 float* t = my_block;
			 for(int i=0;i<counter;i++){

				 if(*v1<*t){
					 memcpy(my_block+length1+length2,my_block,(counter-i)*(length1+length2));
				 	memcpy(my_block,v1,length1);
				 	memcpy(my_block+length1,value2,length2);
				 	return 1;
				 }
				 my_block = my_block + length1 + length2;
				 t=my_block;
			 }
			 memcpy(my_block,v1,length1);
			 memcpy(my_block+length1,value2,length2);
			return 1;
    }else if(type1 == 'i'){
			int* v1 = value1;
			for(int i=0;i<counter;i++){
				if(*v1<*my_block){
					memcpy(my_block+length1+length2,my_block,(counter-i)*(length1+length2));
					memcpy(my_block,v1,length1);
					memcpy(my_block+length1,value2,length2);
					return 1;
				}
				my_block = my_block + length1 + length2;
			}
			memcpy(my_block,v1,length1);
			memcpy(my_block+length1,value2,length2);

			return 1;
	 	 }
	  }

}

int index_insert(char* my_block,void* value1,int pointer,int length1,char* type1){
	int limit = (BF_BLOCK_SIZE - 2*sizeof(int) - sizeof(char))/(length1+sizeof(int));
	int counter;
	memcpy(&counter,my_block+1,sizeof(int));
 	if(counter >= limit){
		return 0;
	}else{
		int temp_counter = counter + 1;
		memcpy(my_block+1,&temp_counter,sizeof(int));//auxanei ton counter
		my_block = my_block +1+2*sizeof(int);

		if(type1 == 'c'){
			char* key = value1;
			for(int i=0;i<counter;i++){
				if(strcmp(key,my_block)<0){
					memcpy(my_block+length1+sizeof(int),my_block,(counter-i)*(sizeof(int)+length1));
					memcpy(my_block,key,length1);
					memcpy(my_block+length1,&pointer,sizeof(int));
					return 1;
				}
				my_block = my_block + length1 + sizeof(int);
			}

			 memcpy(my_block,key,length1);
			 memcpy(my_block+length1,&pointer,sizeof(int));
			 return 1;
		}
		else if(type1 == 'f'){
			float* key = value1;
			float* t = my_block;
			for(int i=0;i<counter;i++){

				if(*key<*t){

					memcpy(my_block+length1+sizeof(int),my_block,(counter-i)*(sizeof(int)+length1));
					memcpy(my_block,key,length1);
					memcpy(my_block+length1,&pointer,sizeof(int));
					return 1;
				}
				my_block = my_block + length1 + sizeof(int);
				t=my_block;
			}
			memcpy(my_block,key,length1);
			memcpy(my_block+length1,&pointer,sizeof(int));
			return 1;

		}else if(type1 == 'i'){
			int* key = value1;
			for(int i=0;i<counter;i++){
				if(*key<*my_block){
					memcpy(my_block+length1+sizeof(int),my_block,(counter-i)*(sizeof(int)+length1));
					memcpy(my_block,key,length1);
					memcpy(my_block+length1,&pointer,sizeof(int));
					return 1;
				}
				my_block = my_block + length1 + sizeof(int);
			}
			memcpy(my_block,key,length1);
			memcpy(my_block+length1,&pointer,sizeof(int));
			return 1;

		}

	}

}



int break_leaf(int fd,char* above_index,char* my_block,void* value1,void* value2,char* type1,int length1,int length2){
  BF_Block *block;
	char* new_block;
	BF_Block_Init(&block);
	BF_AllocateBlock(fd,block);
	new_block = BF_Block_GetData(block);

	int new_pointer;
	BF_GetBlockCounter(fd,&new_pointer);
	new_pointer--; //gia na deixnei sto kainourgio block

	new_block[0] = 'L';

	memcpy(new_block+1+sizeof(int),my_block+1+sizeof(int),sizeof(int)); //vazei ton dikth pou dixnei to aristero sto dexi
	memcpy(my_block+1+sizeof(int),&new_pointer,sizeof(int));

	int temp_counter;
	int my_counter;
	int new_counter;
	memcpy(&temp_counter,my_block+1,sizeof(int));
	my_counter = temp_counter/2;
	new_counter = temp_counter - my_counter;

	int flag;
	char* temp;
	int i=0;
	temp = my_block + 1+ 2*sizeof(int) + (my_counter-1)*(length1+length2);
	do{
		flag = 1;
		if(type1 == 'i'){
			if(*temp == *(my_block+1+2*sizeof(int)+ my_counter*(length1+length2))){

			 flag = 0;
				i++;
				temp = temp - (length1 + length2);
			}
		}else if(type1 == 'c'){
			if(strcmp(temp,my_block+1+2*sizeof(int)+ my_counter*(length1+length2))==0){
				flag = 0;
				i++;
				temp = temp - (length1 + length2);
			}
		}else if(type1 == 'f'){
			// if(*temp == *(my_block+1+2*sizeof(int)+ my_counter*(length1+length2))){
			//
			//  flag = 0;
			// 	i++;
			// 	temp = temp - (length1 + length2);
			// }
		}

	}while(flag == 0);


	my_counter = my_counter - i;
	new_counter = new_counter + i;




	memcpy(my_block+1,&my_counter,sizeof(int)); //alazei counter tou paliou
	memcpy(new_block+1,&new_counter,sizeof(int));// allazei counter neou

	memcpy(new_block+1+2*sizeof(int),my_block+1+2*sizeof(int)+ my_counter*(length1+length2),new_counter*(length1+length2));

	void* key = new_block+1+2*sizeof(int);
	if(type1 == 'c'){

		if(strcmp(value1,key)<0){

			leaf_insert(my_block,value1,value2,type1,length1,length2);
		}else{
			leaf_insert(new_block,value1,value2,type1,length1,length2);
		}
	}else if(type1 == 'i'){
		int* v1 = value1;
		int*v2 = key;
		if(*v1 < *v2){

			leaf_insert(my_block,value1,value2,type1,length1,length2);
		}else{
			leaf_insert(new_block,value1,value2,type1,length1,length2);
		}
	}
	//leaf_insert(my_block,value1,value2,type1,length1,length2);


	index_insert(above_index,key,new_pointer,length1,type1);


	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);

}




int AM_InsertEntry(int fileDesc, void *value1, void *value2) {
	BF_Block *block;
 	char* data;
	char* root; //this will alwas point root
	int blocks_num;
	char type[2]; //this holds the types of value1 and value2
	int length[2]; //this holds the lengths of valuew 1 and value2
	int sizeOfLeaf;//how many entrys can a leaf hold
	int sizeOfIndex;//how many packs of pointer(int)+length[1] can a block of index hold

	BF_Block_Init(&block);
	BF_GetBlock(fileDesc,0,block);
	data = BF_Block_GetData(block);

	memcpy(type,data,sizeof(char));
	memcpy(type+1,data+1,sizeof(char));
	memcpy(length,data+2,sizeof(int));
	memcpy(length+1,data+6,sizeof(int));
	/*
		sizeOfLeaf->se kathe block filo  xwraei enas counter kai sizeofleaf*entrys.oo counter einai gia posa entrys  einai mesa
		sizeofindex->se kathe block tou eurethriou xerane counter kai sizeofleaf*(to megethos tou key + enan int gia dikti) + 1 akoma int
		dikti giati oi diktes einai enas parapanw apo ta key
	*/
	sizeOfLeaf = (BF_BLOCK_SIZE - 2*sizeof(int) - sizeof(char))/(length[0]+length[1]); //to sizeof(int) einai gia ton counter mesa sto block
	sizeOfIndex = (BF_BLOCK_SIZE - 2*sizeof(int) - sizeof(char))/(length[0]+sizeof(int)); // to char einai gia




  BF_GetBlockCounter(fileDesc,&blocks_num);
	if(blocks_num == 1){ //afto tha ginei th prwth fora
		root = init_tree(fileDesc,value1,length[0],type[0]);


		//Afto einai gia to dexi leaf
		BF_AllocateBlock(fileDesc,block);
		data = BF_Block_GetData(block);

		int counter = 1;
		int leaf_pointer = -1;

		data[0] = 'L';
		memcpy(data+1,&counter,sizeof(int));
		memcpy(data+1+sizeof(int),&leaf_pointer,sizeof(int));
		memcpy(data+1+2*sizeof(int),value1,length[0]);
		memcpy(data+1+2*sizeof(int)+length[0],value2,length[1]);
		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);


		//gia to aristero
		BF_AllocateBlock(fileDesc,block);
		data = BF_Block_GetData(block);

		counter = 0;
		leaf_pointer = 2;

		data[0] = 'L';
		memcpy(data+1,&counter,sizeof(int));
		memcpy(data+1+sizeof(int),&leaf_pointer,sizeof(int));
		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);

	  }else{
		BF_GetBlock(fileDesc,1,block);
		root = BF_Block_GetData(block);


		int pointer = scan_index(root,value1,length[0],type[0]);
		//an vrei to leaf

				int isFull;
			  BF_Block *temp_block;
				char* temp_data;

				BF_Block_Init(&temp_block);
				BF_GetBlock(fileDesc,pointer,temp_block);
				temp_data = BF_Block_GetData(temp_block);

	 	    isFull = leaf_insert(temp_data,value1,value2,type[0],length[0],length[1]);


				BF_Block_SetDirty(temp_block);
				BF_UnpinBlock(temp_block);


				if(isFull == 0){

					break_leaf(fileDesc,root,temp_data,value1,value2,type[0],length[0],length[1]);

				}
			//  BF_Block_SetDirty(block);
			// BF_UnpinBlock(block);
	}
  return AME_OK;
}

int integer_search(int v, int length,int length2, int fileDesc, int scanp)
{
	BF_Block *block;
	char* data;
	BF_Block_Init(&block);
	CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc, 1, block));
	data = BF_Block_GetData(block);
    Scan_Array[scanp].record_desc = -1;
    while(data[0] != 'L')
    {
        int maxk;
        memcpy(&maxk,data+1,sizeof(int));
        for(int i=0;i<maxk;i++)
        {
            int temp;
            memcpy(&temp,data+1+2*sizeof(int)+i*(length+sizeof(int)), sizeof(int));
            if(v<temp)
            {
                int new_b;
                memcpy(&new_b, data+1+2*sizeof(int)+i*(length+sizeof(int))-sizeof(int) ,sizeof(int));
                CALL_BF(BF_UnpinBlock(block));
                CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
                data = BF_Block_GetData(block);
                Scan_Array[scanp].block_num = new_b;
                Scan_Array[scanp].first_block = new_b;
                Scan_Array[scanp].last_block = new_b;
                break;
            }
            else if(v>temp)
            {
                if(i == (maxk-1))
                {
                    int new_b;
                    memcpy(&new_b, data+1+2*sizeof(int)+i*(length+sizeof(int))+length ,sizeof(int));
                    CALL_BF(BF_UnpinBlock(block));
                    CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
                    data = BF_Block_GetData(block);
                    Scan_Array[scanp].block_num = new_b;
                    Scan_Array[scanp].first_block = new_b;
                    Scan_Array[scanp].last_block = new_b;
                }
        	}
            else if(v==temp)
            {
                int new_b;
                memcpy(&new_b, data+1+2*sizeof(int)+i*(length+sizeof(int))+length ,sizeof(int));
                CALL_BF(BF_UnpinBlock(block));
                CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
                data = BF_Block_GetData(block);
                Scan_Array[scanp].block_num = new_b;
                Scan_Array[scanp].first_block = new_b;
                Scan_Array[scanp].last_block = new_b;
                break;
            }
        }
    }
    int maxr;
	memcpy(&maxr,data+1,sizeof(int));
    for(int i=0;i<maxr;i++)
    {
        int temp;
        memcpy(&temp,data+1+2*sizeof(int)+i*(length2+sizeof(int)), sizeof(int));
        if(v==temp)
        {
            Scan_Array[scanp].record_desc = i;
            Scan_Array[scanp].record_desc = i;
            break;
        }
    }
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    return Scan_Array[scanp].record_desc;
}

int float_search(float v, int length,int length2, int fileDesc, int scanp)
{
	BF_Block *block;
	char* data;
	BF_Block_Init(&block);
	CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc, 1, block));
	data = BF_Block_GetData(block);
    Scan_Array[scanp].record_desc = -1;
    while(data[0] != 'L')
    {
        int maxk;
        memcpy(&maxk,data+1,sizeof(int));
        for(int i=0;i<maxk;i++)
        {
            float temp;
            memcpy(&temp,data+1+2*sizeof(int)+i*(length+sizeof(int)), sizeof(float));
            if(v<temp)
            {
                int new_b;
                memcpy(&new_b, data+1+2*sizeof(int)+i*(length+sizeof(int))-sizeof(int) ,sizeof(int));
                CALL_BF(BF_UnpinBlock(block));
                CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
                data = BF_Block_GetData(block);
                Scan_Array[scanp].block_num = new_b;
                Scan_Array[scanp].first_block = new_b;
                Scan_Array[scanp].last_block = new_b;
                break;
            }
            else if(v>temp)
            {
                if(i == (maxk-1))
                {
                    int new_b;
                    memcpy(&new_b, data+1+2*sizeof(int)+i*(length+sizeof(int))+length ,sizeof(int));
                    CALL_BF(BF_UnpinBlock(block));
                    CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
                    data = BF_Block_GetData(block);
                    Scan_Array[scanp].block_num = new_b;
                    Scan_Array[scanp].first_block = new_b;
                    Scan_Array[scanp].last_block = new_b;
                }
        	}
            else if(v==temp)
            {
                int new_b;
                memcpy(&new_b, data+1+2*sizeof(int)+i*(length+sizeof(int))+length ,sizeof(int));
                CALL_BF(BF_UnpinBlock(block));
                CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
                data = BF_Block_GetData(block);
                Scan_Array[scanp].block_num = new_b;
                Scan_Array[scanp].first_block = new_b;
                Scan_Array[scanp].last_block = new_b;
                break;
            }
        }
    }
    int maxr;
	memcpy(&maxr,data+1,sizeof(int));
    for(int i=0;i<maxr;i++)
    {
        float temp;
        memcpy(&temp,data+1+2*sizeof(int)+i*(length2+sizeof(int)), sizeof(float));
        if(v==temp)
        {
            Scan_Array[scanp].record_desc = i;
            Scan_Array[scanp].record_desc = i;
            break;
        }
    }
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    return Scan_Array[scanp].record_desc;
}


int AM_OpenIndexScan(int fileDesc, int op, void *value) 
{
	int scanp;
	for(scanp=0;scanp<20;scanp++)
	{
		if(Scan_Array[scanp].block_num == 0)
		{
			Scan_Array[scanp].file_desc = AM_Array[fileDesc].file_desc;
			Scan_Array[scanp].op = op;
			Scan_Array[scanp].block_num = 1;
			Scan_Array[scanp].value = value;
			break;
		}	
	}
	
	char type[2];
	int length[2];
	BF_Block *block;
	char* data;
	BF_Block_Init(&block);
	CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc, 0, block));
	data = BF_Block_GetData(block);
	type[0] = data[0];
	type[1] = data[1];
	memcpy(length,data+2,sizeof(int));
	memcpy(length+1,data+2+sizeof(int),sizeof(int));
	Scan_Array[scanp].type = type[0];
	CALL_BF(BF_UnpinBlock(block));
	
	int temp_rd;
	if(type[0]== 'i')
	{
		int v;
		v = *((int*)value);
        temp_rd = integer_search(v,length[0],length[1],fileDesc,scanp);
        if(op == EQUAL)
        	return scanp;
        else
        {
        	CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc, 1, block));
        	data = BF_Block_GetData(block);
        	
        	int fchild,childp,prev_child;
			while(data[0] != 'L')
			{
				memcpy(&fchild,data+1+sizeof(int),sizeof(int));
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,fchild,block));
				data = BF_Block_GetData(block);
			}
			if(op == NOT_EQUAL)
			{
				Scan_Array[scanp].first_block = fchild;
				Scan_Array[scanp].record_desc = 0;
				Scan_Array[scanp].last_block = -1;
			}
			else if(op == LESS_THAN || op == LESS_THAN_OR_EQUAL)
			{
				Scan_Array[scanp].first_block = fchild;
				Scan_Array[scanp].record_desc = 0;
				Scan_Array[scanp].last_block = Scan_Array[scanp].block_num;
			}
			else
			{
				Scan_Array[scanp].first_block = Scan_Array[scanp].block_num;
				Scan_Array[scanp].last_block = -1;
			}
			CALL_BF(BF_UnpinBlock(block));
			BF_Block_Destroy(&block);
			return scanp;
		}
    }
    else if(type[0]=='f')
    {
		float v;
		v = *((float*)value);
        temp_rd = float_search(v,length[0],length[1],fileDesc,scanp);
        if(op == EQUAL)
        	return scanp;
        else
        {
        	CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc, 1, block));
        	data = BF_Block_GetData(block);
        	
        	int fchild,childp,prev_child;
			while(data[0] != 'L')
			{
				memcpy(&fchild,data+1+sizeof(int),sizeof(int));
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,fchild,block));
				data = BF_Block_GetData(block);
			}
			if(op == NOT_EQUAL)
			{
				Scan_Array[scanp].first_block = fchild;
				Scan_Array[scanp].record_desc = 0;
				Scan_Array[scanp].last_block = -1;
			}
			else if(op == LESS_THAN || op == LESS_THAN_OR_EQUAL)
			{
				Scan_Array[scanp].first_block = fchild;
				Scan_Array[scanp].record_desc = 0;
				Scan_Array[scanp].last_block = Scan_Array[scanp].block_num;
			}
			else
			{
				Scan_Array[scanp].first_block = Scan_Array[scanp].block_num;
				Scan_Array[scanp].last_block = -1;
			}
			CALL_BF(BF_UnpinBlock(block));
			BF_Block_Destroy(&block);
			return scanp;
		}    	
	}
	
	CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc, 1, block));
	data = BF_Block_GetData(block);
	
	if(op == EQUAL)
	{
		Scan_Array[scanp].record_desc = -1;
		while(data[0] != 'L')
		{
			int maxk;
			memcpy(&maxk,data+1,sizeof(int));
			for(int i=0;i<maxk;i++)
			{
				if(memcmp(value, data+1+2*sizeof(int)+i*(length[0]+sizeof(int)) ,sizeof(value))<0)
				{
					int new_b;
					memcpy(&new_b, data+1+2*sizeof(int)+i*(length[0]+sizeof(int))-sizeof(int) ,sizeof(int));
					CALL_BF(BF_UnpinBlock(block));
					CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
					data = BF_Block_GetData(block);
					Scan_Array[scanp].block_num = new_b;
					Scan_Array[scanp].first_block = new_b;
					Scan_Array[scanp].last_block = new_b;
					break;	
				}
				else if(memcmp(value, data+1+2*sizeof(int)+i*(length[0]+sizeof(int)) ,sizeof(value))>0)
				{
					if(i == (maxk-1))
					{
						int new_b;
						memcpy(&new_b, data+1+2*sizeof(int)+i*(length[0]+sizeof(int))+length[0] ,sizeof(int));
						CALL_BF(BF_UnpinBlock(block));
						CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
						data = BF_Block_GetData(block);
						Scan_Array[scanp].block_num = new_b;
						Scan_Array[scanp].first_block = new_b;
						Scan_Array[scanp].last_block = new_b;
					}
				}
				else if(memcmp(value, data+1+2*sizeof(int)+i*(length[0]+sizeof(int)) ,sizeof(value))==0)
				{
					int new_b;
					memcpy(&new_b, data+1+2*sizeof(int)+i*(length[0]+sizeof(int))+length[0] ,sizeof(int));
					CALL_BF(BF_UnpinBlock(block));
					CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
					data = BF_Block_GetData(block);
					Scan_Array[scanp].block_num = new_b;
					Scan_Array[scanp].first_block = new_b;
					Scan_Array[scanp].last_block = new_b;
					break;
				}
			}
		}
		int maxr;
		memcpy(&maxr,data+1,sizeof(int));
		for(int i=0;i<maxr;i++)
		{
			if(memcmp(value,data+1+2*sizeof(int)+i*(length[0]+length[1]),sizeof(value))== 0)
			{
				Scan_Array[scanp].record_desc = i;
				break;
			}	
		}
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		printf("first block = %d\n, last block = %d\n, block_num = %d\n record desc = %d\n",Scan_Array[scanp].first_block,Scan_Array[scanp].last_block,Scan_Array[scanp].block_num,Scan_Array[scanp].record_desc);
		return scanp;
	}
	
	else if(op == NOT_EQUAL)
	{
		int fchild;
		while(data[0] != 'L')
		{
			memcpy(&fchild,data+1+sizeof(int),sizeof(int));
			CALL_BF(BF_UnpinBlock(block));
			CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,fchild,block));
			data = BF_Block_GetData(block);
		}
		Scan_Array[scanp].first_block = fchild;
		Scan_Array[scanp].last_block = -1;
		int childp,first,prev_child;
		first=1;
		memcpy(&childp,data+1+sizeof(int),sizeof(int));
		while(childp!=-1)
		{
			int count;
			int flag=0;
			memcpy(&count,data+1,sizeof(int));
			for(int i=0;i<count;i++)
			{
				if(memcmp(value,data+1+2*sizeof(int)+i*(length[0]+length[1]),sizeof(value))== 0)
				{
					if(first)
					{
						Scan_Array[scanp].block_num = fchild;
						Scan_Array[scanp].first_block = childp;
						first=0;
					}
					else
						Scan_Array[scanp].block_num = prev_child;
					CALL_BF(BF_UnpinBlock(block));
					CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,childp,block));
					data = BF_Block_GetData(block);
					memcpy(&childp,data+1+sizeof(int),sizeof(int));
					flag=1;
					break;
				}	
			}
			if(!flag)
			{
				first=0;
				prev_child = childp;
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,childp,block));
				data = BF_Block_GetData(block);
				memcpy(&childp,data+1+sizeof(int),sizeof(int));
			}
		}
		int count;
		memcpy(&count,data+1,sizeof(int));
		for(int i=0;i<count;i++)
		{
			if(memcmp(value,data+1+2*sizeof(int)+i*(length[0]+length[1]),sizeof(value))== 0)
			{
				Scan_Array[scanp].block_num = -1;
				Scan_Array[scanp].last_block = prev_child;
				break;
			}
		}
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		Scan_Array[scanp].record_desc = 0;
		return scanp;	
	}
	
	else if(op == LESS_THAN || op == LESS_THAN_OR_EQUAL)
	{
		int fchild;
		while(data[0] != 'L')
		{
			memcpy(&fchild,data+1+sizeof(int),sizeof(int));
			CALL_BF(BF_UnpinBlock(block));
			CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,fchild,block));
			data = BF_Block_GetData(block);
		}
		Scan_Array[scanp].block_num = fchild;
		int childp;
		int prev_child=fchild;
		memcpy(&childp,data+1+sizeof(int),sizeof(int));
		CALL_BF(BF_UnpinBlock(block));
		Scan_Array[scanp].first_block = fchild;
		while(childp!=-1)
		{
			int count;
			int flag=0;
			memcpy(&count,data+1,sizeof(int));
			for(int i=0;i<count;i++)
			{
				if(memcmp(value,data+1+2*sizeof(int)+i*(length[0]+length[1]),sizeof(value))== 0)
				{
					Scan_Array[scanp].block_num = prev_child;
					Scan_Array[scanp].record_desc = 0;
					Scan_Array[scanp].last_block = prev_child;
					CALL_BF(BF_UnpinBlock(block));
					flag=1;
					break;
				}
			}
			if(!flag)
			{
				prev_child = childp;
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,childp,block));
				data = BF_Block_GetData(block);
				memcpy(&childp,data+1+sizeof(int),sizeof(int));
			}
			else
			{
				BF_Block_Destroy(&block);
				return scanp;
			}
		}
		int count;
		memcpy(&count,data+1,sizeof(int));
		for(int i=0;i<count;i++)
		{
			if(memcmp(value,data+1+2*sizeof(int)+i*(length[0]+length[1]),sizeof(value))== 0)
			{
				Scan_Array[scanp].block_num = prev_child;
				Scan_Array[scanp].record_desc = 0;
				Scan_Array[scanp].last_block = prev_child;
				CALL_BF(BF_UnpinBlock(block));
				break;
			}
		}
		BF_Block_Destroy(&block);
		return scanp;
	}
	
	else
	{
		while(data[0] != 'L')
		{
			int maxk;
			memcpy(&maxk,data+1,sizeof(int));
			for(int i=0;i<maxk;i++)
			{
				if(memcmp(value, data+1+2*sizeof(int)+i*(length[0]+sizeof(int)) ,sizeof(value))<0)
				{
					int new_b;
					memcpy(&new_b, data+1+2*sizeof(int)+i*(length[0]+sizeof(int))-sizeof(int) ,sizeof(int));
					CALL_BF(BF_UnpinBlock(block));
					CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
					data = BF_Block_GetData(block);
					Scan_Array[scanp].block_num = new_b;
					Scan_Array[scanp].first_block = new_b;
					Scan_Array[scanp].last_block = -1;
					break;	
				}
				else if(memcmp(value, data+1+2*sizeof(int)+i*(length[0]+sizeof(int)) ,sizeof(value))>0)
				{
					if(i == (maxk-1))
					{
						int new_b;
						memcpy(&new_b, data+1+2*sizeof(int)+i*(length[0]+sizeof(int))+length[0] ,sizeof(int));
						CALL_BF(BF_UnpinBlock(block));
						CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
						data = BF_Block_GetData(block);
						Scan_Array[scanp].block_num = new_b;
						Scan_Array[scanp].first_block = new_b;
						Scan_Array[scanp].last_block = -1;
					}
				}
				else if(memcmp(value, data+1+2*sizeof(int)+i*(length[0]+sizeof(int)) ,sizeof(value))==0)
				{
					int new_b;
					memcpy(&new_b, data+1+2*sizeof(int)+i*(length[0]+sizeof(int))+length[0] ,sizeof(int));
					CALL_BF(BF_UnpinBlock(block));
					CALL_BF(BF_GetBlock(AM_Array[fileDesc].file_desc,new_b , block));
					data = BF_Block_GetData(block);
					Scan_Array[scanp].block_num = new_b;
					Scan_Array[scanp].record_desc = 0;
					Scan_Array[scanp].first_block = new_b;
					Scan_Array[scanp].last_block = -1;
					break;
				}
			}
		}
		int maxr;
		memcpy(&maxr,data+1,sizeof(int));
		for(int i=0;i<maxr;i++)
		{
			if(memcmp(value,data+1+2*sizeof(int)+i*(length[0]+length[1]),sizeof(value))== 0)
			{
				Scan_Array[scanp].record_desc = i;
				break;
			}	
		}
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		return scanp;
	}
}

void* integer_nextEntry(int scanDesc,int length1, int length2)
{
	BF_Block *block;
	char* data;
	int value;
	void* val;
	BF_Block_Init(&block);
	
	if(Scan_Array[scanDesc].op == EQUAL)
	{
		if(Scan_Array[scanDesc].record_desc == -1)
		{
			AM_errno = AME_EOF;
			return NULL;
		}
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].block_num,block));
		data = BF_Block_GetData(block);
		int rd = Scan_Array[scanDesc].record_desc;
		memcpy(&value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		val = &value;
		return val;
	}
	else if(Scan_Array[scanDesc].op == NOT_EQUAL)
	{
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].first_block,block));
		data = BF_Block_GetData(block);
		int count,next_block;
		memcpy(&count,data+1,sizeof(int));
		memcpy(&next_block,data+1+sizeof(int),sizeof(int));
		int rd = Scan_Array[scanDesc].record_desc;
		if(rd == count)
		{
			rd=0;
			Scan_Array[scanDesc].record_desc = rd;
			if(next_block == Scan_Array[scanDesc].last_block)
			{
				AM_errno = AME_EOF;
				return NULL;
			}
			else
			{
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,next_block,block));
				data = BF_Block_GetData(block);
				Scan_Array[scanDesc].first_block = next_block;
			}
		}
		memcpy(&count,data+1,sizeof(int));
		if(Scan_Array[scanDesc].first_block == Scan_Array[scanDesc].block_num)
		{
			for(int i=0;i<count;i++)
			{
				if(Scan_Array[scanDesc].type == 'c')
				{
				
					if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))==0)
					{
						rd++;
					}
				}
				else
				{
					float v = *((float*)Scan_Array[scanDesc].value);
					float check;
					memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
					if(v == check)
						rd++;
				}
			}
		}	
		memcpy(&value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		rd++;
		Scan_Array[scanDesc].record_desc = rd;
		val = &value;
		return val;
	}
	else if(Scan_Array[scanDesc].op == LESS_THAN || Scan_Array[scanDesc].op == LESS_THAN_OR_EQUAL)
	{
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].first_block,block));
		data = BF_Block_GetData(block);
		int count,next_block;
		memcpy(&count,data+1,sizeof(int));
		memcpy(&next_block,data+1+sizeof(int),sizeof(int));
		int rd = Scan_Array[scanDesc].record_desc;
		if(rd == count)
		{
			rd=0;
			Scan_Array[scanDesc].record_desc = rd;
			if(next_block == Scan_Array[scanDesc].last_block)
			{
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,next_block,block));
				data = BF_Block_GetData(block);
				Scan_Array[scanDesc].first_block = next_block;
				memcpy(&count,data+1,sizeof(int));
				if(Scan_Array[scanDesc].op == LESS_THAN)
				{	
					for(int i=0;i<count;i++)
					{
						if(Scan_Array[scanDesc].type == 'c')
						{
							if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))==0)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
						else
						{
							float v = *((float*)Scan_Array[scanDesc].value);
							float check;
							memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
							if(v==check)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
					}
				}
				else
				{
					for(int i=0;i<count;i++)
					{
						if(Scan_Array[scanDesc].type == 'c')
						{
							if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))<0)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
						else
						{
							float v = *((float*)Scan_Array[scanDesc].value);
							float check;
							memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
							if(v<check)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
					}
				}	
			}
		}
		memcpy(&count,data+1,sizeof(int));
		if(Scan_Array[scanDesc].first_block == Scan_Array[scanDesc].block_num)
		{
			if(Scan_Array[scanDesc].op == LESS_THAN)
			{	
				for(int i=0;i<count;i++)
				{
					if(Scan_Array[scanDesc].type == 'c')
					{
						if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))==0)
						{	
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
					else
					{
						float v = *((float*)Scan_Array[scanDesc].value);
						float check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
						if(v==check)
						{
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
				}
			}
			else
			{
				for(int i=0;i<count;i++)
				{
					if(Scan_Array[scanDesc].type == 'c')
					{
						if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))<0)
						{
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
					else
					{
						float v = *((float*)Scan_Array[scanDesc].value);
						float check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
						if(v<check)
						{
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
				}
			}
		}	
		memcpy(&value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		rd++;
		Scan_Array[scanDesc].record_desc = rd;
		val = &value;
		return val;
	}
	else if(Scan_Array[scanDesc].op == GREATER_THAN || Scan_Array[scanDesc].op == GREATER_THAN_OR_EQUAL)
	{
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].first_block,block));
		data = BF_Block_GetData(block);
		int count,next_block;
		memcpy(&count,data+1,sizeof(int));
		memcpy(&next_block,data+1+sizeof(int),sizeof(int));
		int rd = Scan_Array[scanDesc].record_desc;
		if(Scan_Array[scanDesc].first_block == Scan_Array[scanDesc].block_num)
		{
			if(Scan_Array[scanDesc].op == GREATER_THAN)
			{
				for(int i=0;i<count;i++)
				{
					if(Scan_Array[scanDesc].type == 'c')
					{
						if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))==0)
							rd++;
					}
					else
					{
						float v = *((float*)Scan_Array[scanDesc].value);
						float check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
						if(v==check)
							rd++;
					}
				}
			}
			else
			{
				for(int i=0;i<count;i++)
				{
					if(Scan_Array[scanDesc].type == 'c')
					{	
						if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))<0)
							rd++;
					}
					else
					{
						float v = *((float*)Scan_Array[scanDesc].value);
						float check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
						if(v<check)
							rd++;
					}
				}
			}
		}
		if(rd == count)
		{
			rd=0;
			Scan_Array[scanDesc].record_desc = rd;
			if(next_block == Scan_Array[scanDesc].last_block)
			{
				AM_errno = AME_EOF;
				return NULL;
			}
			else
			{
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,next_block,block));
				data = BF_Block_GetData(block);
				Scan_Array[scanDesc].first_block = next_block;
			}
		}
		memcpy(&value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		rd++;
		Scan_Array[scanDesc].record_desc = rd;
		val = &value;
		return val;		
	}
}

void *string_nextEntry(int scanDesc,int length1, int length2)
{
	BF_Block *block;
	char* data;
	void* val;
	char value[length2];
	BF_Block_Init(&block);
	
	if(Scan_Array[scanDesc].op == EQUAL)
	{
		if(Scan_Array[scanDesc].record_desc == -1)
		{
			AM_errno = AME_EOF;
			return NULL;
		}
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].block_num,block));
		data = BF_Block_GetData(block);
		int rd = Scan_Array[scanDesc].record_desc;
		memcpy(value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		val = value;
		return val;
	}
	else if(Scan_Array[scanDesc].op == NOT_EQUAL)
	{
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].first_block,block));
		data = BF_Block_GetData(block);
		int count,next_block;
		memcpy(&count,data+1,sizeof(int));	
		memcpy(&next_block,data+1+sizeof(int),sizeof(int));
		int rd = Scan_Array[scanDesc].record_desc;
		if(rd == count)
		{
			rd=0;
			Scan_Array[scanDesc].record_desc = rd;
			if(next_block == Scan_Array[scanDesc].last_block)
			{
				AM_errno = AME_EOF;
				return NULL;
			}
			else
			{
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,next_block,block));
				data = BF_Block_GetData(block);
				Scan_Array[scanDesc].first_block = next_block;
			}
		}
		memcpy(&count,data+1,sizeof(int));
		if(Scan_Array[scanDesc].first_block == Scan_Array[scanDesc].block_num)
		{
			for(int i=0;i<count;i++)
			{
				if(Scan_Array[scanDesc].type == 'i')
				{
					int v = *((int*)Scan_Array[scanDesc].value);
					int check;
					memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
					if(v == check)
						rd++;
				}
				else
				{
					float v = *((float*)Scan_Array[scanDesc].value);
					float check;
					memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
					if(v == check)
						rd++;
				}
			}
		}
		memcpy(value,data+1+2*sizeof(int)+length1+rd*(length1+length2),/*length2*/sizeof(value));
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		rd++;
		Scan_Array[scanDesc].record_desc = rd;
		val = value;
		return val;
	}
	else if(Scan_Array[scanDesc].op == LESS_THAN || Scan_Array[scanDesc].op == LESS_THAN_OR_EQUAL)
	{
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].first_block,block));
		data = BF_Block_GetData(block);
		int count,next_block;
		memcpy(&count,data+1,sizeof(int));
		memcpy(&next_block,data+1+sizeof(int),sizeof(int));
		int rd = Scan_Array[scanDesc].record_desc;
		if(rd == count)
		{
			rd=0;
			Scan_Array[scanDesc].record_desc = rd;
			if(next_block == Scan_Array[scanDesc].last_block)
			{
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,next_block,block));
				data = BF_Block_GetData(block);
				Scan_Array[scanDesc].first_block = next_block;
				memcpy(&count,data+1,sizeof(int));
				if(Scan_Array[scanDesc].op == LESS_THAN)
				{
					
					for(int i=0;i<count;i++)
					{
						if(Scan_Array[scanDesc].type == 'i')
						{
							int v = *((int*)Scan_Array[scanDesc].value);
							int check;
							memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
							if(v == check)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
						else
						{
							float v = *((float*)Scan_Array[scanDesc].value);
							float check;
							memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
							if(v == check)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}	
						}
					}
				}
				else
				{
					for(int i=0;i<count;i++)
					{
						if(Scan_Array[scanDesc].type == 'i')
						{
							int v = *((int*)Scan_Array[scanDesc].value);
							int check;
							memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
							if(v < check)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
						else
						{
							float v = *((float*)Scan_Array[scanDesc].value);
							float check;
							memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
							if(v<check)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
					}
				}	
			}
		}
		memcpy(&count,data+1,sizeof(int));
		if(Scan_Array[scanDesc].first_block == Scan_Array[scanDesc].block_num)
		{
			if(Scan_Array[scanDesc].op == LESS_THAN)
			{	
				for(int i=0;i<count;i++)
				{
					if(Scan_Array[scanDesc].type == 'i')
					{
						int v = *((int*)Scan_Array[scanDesc].value);
						int check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
						if(v == check)
						{	
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
					else
					{
						float v = *((float*)Scan_Array[scanDesc].value);
						float check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
						if(v == check)
						{
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}	
					}
				}
			}
			else
			{
				for(int i=0;i<count;i++)
				{
					if(Scan_Array[scanDesc].type == 'i')
					{
						int v = *((int*)Scan_Array[scanDesc].value);
						int check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
						if(v < check)
						{
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
					else
					{
						float v = *((float*)Scan_Array[scanDesc].value);
						float check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
						if(v<check)
						{
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
				}
			}
		}	
		memcpy(value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		rd++;
		Scan_Array[scanDesc].record_desc = rd;
		val = value;
		return val;
	}
	else if(Scan_Array[scanDesc].op == GREATER_THAN || Scan_Array[scanDesc].op == GREATER_THAN_OR_EQUAL)
	{
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].first_block,block));
		data = BF_Block_GetData(block);
		int count,next_block;
		memcpy(&count,data+1,sizeof(int));
		memcpy(&next_block,data+1+sizeof(int),sizeof(int));
		int rd = Scan_Array[scanDesc].record_desc;
		if(Scan_Array[scanDesc].first_block == Scan_Array[scanDesc].block_num)
		{
			if(Scan_Array[scanDesc].op == GREATER_THAN)
			{
				for(int i=0;i<count;i++)
				{	
					if(Scan_Array[scanDesc].type == 'i')
					{
						int v = *((int*)Scan_Array[scanDesc].value);
						int check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
						if(v == check)
							rd++;
					}
					else
					{
						float v = *((float*)Scan_Array[scanDesc].value);
						float check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
						if(v == check)
							rd++;
					}
				}
			}
			else
			{
				for(int i=0;i<count;i++)
				{	
					if(Scan_Array[scanDesc].type == 'i')
					{
						int v = *((int*)Scan_Array[scanDesc].value);
						int check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
						if(v < check)
							rd++;
					}
					else
					{
						float v = *((float*)Scan_Array[scanDesc].value);
						float check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(float));
						if(v == check)
							rd++;
					}
				}
			}
		}
		if(rd == count)
		{
			rd=0;
			Scan_Array[scanDesc].record_desc = rd;
			if(next_block == Scan_Array[scanDesc].last_block)
			{
				AM_errno = AME_EOF;
				return NULL;
			}
			else
			{
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,next_block,block));
				data = BF_Block_GetData(block);
				Scan_Array[scanDesc].first_block = next_block;
			}
		}
		memcpy(value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		rd++;
		Scan_Array[scanDesc].record_desc = rd;
		val = value;
		return val;		
	}
}

void *float_nextEntry(int scanDesc,int length1, int length2)
{
	BF_Block *block;
	char* data;
	float value;
	void* val;
	BF_Block_Init(&block);
	
	if(Scan_Array[scanDesc].op == EQUAL)
	{
		if(Scan_Array[scanDesc].record_desc == -1)
		{
			AM_errno = AME_EOF;
			return NULL;
		}
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].block_num,block));
		data = BF_Block_GetData(block);
		int rd = Scan_Array[scanDesc].record_desc;
		memcpy(&value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		val = &value;
		return val;
	}
	else if(Scan_Array[scanDesc].op == NOT_EQUAL)
	{
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].first_block,block));
		data = BF_Block_GetData(block);
		int count,next_block;
		memcpy(&count,data+1,sizeof(int));
		memcpy(&next_block,data+1+sizeof(int),sizeof(int));
		int rd = Scan_Array[scanDesc].record_desc;
		if(rd == count)
		{
			rd=0;
			Scan_Array[scanDesc].record_desc = rd;
			if(next_block == Scan_Array[scanDesc].last_block)
			{
				AM_errno = AME_EOF;
				return NULL;
			}
			else
			{
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,next_block,block));
				data = BF_Block_GetData(block);
				Scan_Array[scanDesc].first_block = next_block;
			}
		}
		memcpy(&count,data+1,sizeof(int));
		if(Scan_Array[scanDesc].first_block == Scan_Array[scanDesc].block_num)
		{
			for(int i=0;i<count;i++)
			{
				if(Scan_Array[scanDesc].type == 'c')
				{
				
					if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))==0)
						rd++;
				}
				else
				{
					int v = *((int*)Scan_Array[scanDesc].value);
					int check;
					memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
					if(v == check)
						rd++;
				}
			}
		}	
		memcpy(&value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		rd++;
		Scan_Array[scanDesc].record_desc = rd;
		val = &value;
		return val;
	}
	else if(Scan_Array[scanDesc].op == LESS_THAN || Scan_Array[scanDesc].op == LESS_THAN_OR_EQUAL)
	{
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].first_block,block));
		data = BF_Block_GetData(block);
		int count,next_block;
		memcpy(&count,data+1,sizeof(int));
		memcpy(&next_block,data+1+sizeof(int),sizeof(int));
		int rd = Scan_Array[scanDesc].record_desc;
		if(rd == count)
		{
			rd=0;
			Scan_Array[scanDesc].record_desc = rd;
			if(next_block == Scan_Array[scanDesc].last_block)
			{
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,next_block,block));
				data = BF_Block_GetData(block);
				Scan_Array[scanDesc].first_block = next_block;
				memcpy(&count,data+1,sizeof(int));
				if(Scan_Array[scanDesc].op == LESS_THAN)
				{
					
					for(int i=0;i<count;i++)
					{
						if(Scan_Array[scanDesc].type == 'c')
						{
							if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))==0)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
						else
						{
							int v = *((int*)Scan_Array[scanDesc].value);
							int check;
							memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
							if(v == check)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
					}
				}
				else
				{
					for(int i=0;i<count;i++)
					{
						if(Scan_Array[scanDesc].type == 'c')
						{
							if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))<0)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
						else
						{
							int v = *((int*)Scan_Array[scanDesc].value);
							int check;
							memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
							if(v < check)
							{
								AM_errno = AME_EOF;
								CALL_BF(BF_UnpinBlock(block));
								BF_Block_Destroy(&block);
								return NULL;
							}
						}
					}
				}	
			}
		}
		memcpy(&count,data+1,sizeof(int));
		if(Scan_Array[scanDesc].first_block == Scan_Array[scanDesc].block_num)
		{
			if(Scan_Array[scanDesc].op == LESS_THAN)
			{	
				for(int i=0;i<count;i++)
				{
					if(Scan_Array[scanDesc].type == 'c')
					{
						if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))==0)
						{	
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
					else
					{
						int v = *((int*)Scan_Array[scanDesc].value);
						int check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
						if(v == check)
						{
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
				}
			}
			else
			{
				for(int i=0;i<count;i++)
				{
					if(Scan_Array[scanDesc].type == 'c')
					{
						if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))<0)
						{
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
					else
					{
						int v = *((int*)Scan_Array[scanDesc].value);
						int check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
						if(v < check)
						{
							AM_errno = AME_EOF;
							CALL_BF(BF_UnpinBlock(block));
							BF_Block_Destroy(&block);
							return NULL;
						}
					}
				}
			}
		}	
		memcpy(&value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		rd++;
		Scan_Array[scanDesc].record_desc = rd;
		val = &value;
		return val;
	}
	else if(Scan_Array[scanDesc].op == GREATER_THAN || Scan_Array[scanDesc].op == GREATER_THAN_OR_EQUAL)
	{
		CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,Scan_Array[scanDesc].first_block,block));
		data = BF_Block_GetData(block);
		int count,next_block;
		memcpy(&count,data+1,sizeof(int));
		memcpy(&next_block,data+1+sizeof(int),sizeof(int));
		int rd = Scan_Array[scanDesc].record_desc;
		if(Scan_Array[scanDesc].first_block == Scan_Array[scanDesc].block_num)
		{
			if(Scan_Array[scanDesc].op == GREATER_THAN)
			{
				for(int i=0;i<count;i++)
				{
					if(Scan_Array[scanDesc].type == 'c')
					{
						if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))==0)
							rd++;
					}
					else
					{
						int v = *((int*)Scan_Array[scanDesc].value);
						int check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
						if(v==check)
							rd++;
					}
				}
			}
			else
			{
				for(int i=0;i<count;i++)
				{	
					if(Scan_Array[scanDesc].type == 'c')
					{
						if(memcmp(Scan_Array[scanDesc].value,data+1+2*sizeof(int)+i*(length1+length2),sizeof(Scan_Array[scanDesc].value))<0)
							rd++;
					}
					else
					{
						int v = *((int*)Scan_Array[scanDesc].value);
						int check;
						memcpy(&check,data+1+2*sizeof(int)+i*(length1+length2),sizeof(int));
						if(v<check)
							rd++;	
					}
				}
			}
		}
		if(rd == count)
		{
			rd=0;
			Scan_Array[scanDesc].record_desc = rd;
			if(next_block == Scan_Array[scanDesc].last_block)
			{
				AM_errno = AME_EOF;
				return NULL;
			}
			else
			{
				CALL_BF(BF_UnpinBlock(block));
				CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc,next_block,block));
				data = BF_Block_GetData(block);
				Scan_Array[scanDesc].first_block = next_block;
			}
		}
		memcpy(&value,data+1+2*sizeof(int)+length1+rd*(length1+length2),length2);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		rd++;
		Scan_Array[scanDesc].record_desc = rd;
		val = &value;
		return val;		
	}
}

void *AM_FindNextEntry(int scanDesc) 
{
	if(Scan_Array[scanDesc].block_num == 0)
	{
		AM_errno= -2;
		return NULL;
	}
	
	BF_Block *block;
	char* data;
	BF_Block_Init(&block);
	CALL_BF(BF_GetBlock(Scan_Array[scanDesc].file_desc, 0, block));
	data = BF_Block_GetData(block);
	char type[2];
	int length[2];
	type[0] = data[0];
	type[1] = data[1];
	memcpy(length,data+2,sizeof(int));
	memcpy(length+1,data+2+sizeof(int),sizeof(int));
	CALL_BF(BF_UnpinBlock(block));
	
	void* value;
	if(type[1] == 'i')
	{ 
		value = integer_nextEntry(scanDesc,length[0],length[1]);
		BF_Block_Destroy(&block);
		return value;
	}
	else if(type[1] == 'c')
	{
		value = string_nextEntry(scanDesc,length[0],length[1]);
		BF_Block_Destroy(&block);
		return value;
	}
	else if(type[1] == 'f')
	{
		float v;
		value = float_nextEntry(scanDesc,length[0],length[1]);
		BF_Block_Destroy(&block);
		return value;
	}
}


int AM_CloseIndexScan(int scanDesc) {
  return AME_OK;
}


void AM_PrintError(char *errString) 
{
//	if(errString != NULL) printf("%s\n",errString);
//    switch (AM_errno)
//    {
//        case AME_EOF:
//            printf("\nEnd of file has been reached\n");
//            break;
// 
//        case AME_ERROR:
//            printf("\nError\n");
//            break;
// 
//        case AME_FILE_NOT_FOUND:
//            printf("\nThe specified file was not found\n");
//            break;
// 
//        case AME_NO_FILE_SPACE:
//            printf("\nNo space to open another file\n");
//            break;
// 
//        case AME_WRONG_FILE_TYPE:
//            printf("\nFile was not an Access Method file\n");
//            break;
// 
//        case AME_BF_ERROR:
//            printf("\nError in BF level\n");
//            break;
// 
//        case AME_SCAN_NOT_FOUND:
//            printf("\nThe specified scan was not found\n");
//            break;
// 
//        case AME_TYPE_LENGTH_MISMATCH:
//            printf("\nThe type and the length given do not match\n");
//            break;
//            
//		case AME_FILE_IS_OPEN:
//			printf("\nThe specified file is open\n");
//			break;
//			
//		case AME_INSERT_ERROR:
//			printf("\nINSERTION FAULT \n");
//			break;
//			
//		case AME_FILE_ALREADY_EXISTS:
//			printf("\nCannot create file. File already exists\n");
//			break;
//			
//		case AME_NO_SCAN_SPACE:
//			printf("\nCannot open scan. No space for another scan\n");
//			break;
//			
//	//	case AME_BLOCK_NOT_FOUND:
//	//		printf("\n The value doesnt match to any block in the file.\n");
//	//		break;
//        default:
//            printf("\nUnknown error type\n");
//            break;		
//    }  
}

void AM_Close() {
  
}
