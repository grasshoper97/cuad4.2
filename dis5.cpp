// usage: ./a.out  infile  outfile
// 2015.08.05
// simulate a cache (32 set, 128B line size , 4 assoc),get the RD of every  address in every set( the num of address in every set may diffirent)
// 2015.08.06 merge the address in the same line(128B) as the same address
#include <iostream>
#include<string.h>
#include <stdio.h>  
#include <list>
#include <vector>
#include <numeric>
#include <algorithm>
#define SET_NUM 32
#define ASSO_NUM 90 
#define GTX480_ASSO 4
#define BLOCK_BYPE 128
#define DISTANT  128
using namespace std;
//-----------------------------------------------------------------
class LRUcache{
	private:
		vector< list<unsigned int> > cache ;
	public:
		long long int hit_num;
		long long int miss_num;
		LRUcache(){
			hit_num=0;
			miss_num=0;
			for(int i=0;i<SET_NUM;i++)
				cache.push_back( list<unsigned int>() );
		}
		//................................................
		void show_cache()
		{
			list<unsigned int>::iterator it;
			for(int i=0;i<SET_NUM; i++){
				cout<< "set["<<i<<"]   ";
				for(it= cache[i].begin(); it != cache[i].end() ; it++)
					cout<< *it << ",";
				cout << endl;
			}

		}
		//................................................
		int access_set(int setNo ,unsigned int merged_addr, unsigned old_addr)
		{
			list<unsigned int>::iterator it;
			int RD=-1,i=0;
			for(it=cache[setNo].begin(); it != cache[setNo].end() ; it++ , i++)
			{	
				// hit 
				if( *it == merged_addr) 
				{	//record the position as RD;
					if( i< GTX480_ASSO )
						hit_num++;  // the GTX480 asso=4, this app set asso=90 to gather the RD
					else 
						miss_num++;
					RD=i;
					// promote this element as exit;
					cache[setNo].erase(it);
					cache[setNo].push_front(merged_addr);
					// exit;
					break;
				}
			}
			// miss ,set to DISTANT
			if(it == cache[setNo].end()){
				miss_num++;
				RD = DISTANT;
				// cache is not full, insert to head
				if(  cache[setNo].size() < ASSO_NUM )
					cache[setNo].push_front(merged_addr);
				else{
					// cache is full, romove the last,and insert to head
					cache[setNo].pop_back();
					cache[setNo].push_front(merged_addr);
				}
			}
			// addr  merged_addr setNo PD
			printf("%x %X  set=%d RD=%d \n",old_addr,merged_addr,setNo,RD);
			return RD;
		}
		//................................................
		int access_cache (unsigned int addr)
		{
			unsigned int merge_addr=( addr>>7 )<< 7; // 地址对其到line
			unsigned int setNo=( (addr -0x80000000 )/128 )% SET_NUM;
			return access_set( setNo ,merge_addr,addr);
		}
};
//-----------------------------------------------------------------
int main(int argc, char** argv)  
{  
	if(argc<3){
		printf("no 3 files specfied! ");
		exit(0);
	}
	else printf("%s,%s",argv[1],argv[2]);
	unsigned int addr;  

	FILE *in=NULL, *out =NULL , *hit_rate= NULL;  
	if ( ( in=fopen(argv[1],"r") ) == NULL) {//以只读方式打开源文件  

		printf("Can't open the in file!\n"); 
		exit(0);
	}
	if( (out = fopen(argv[2],"wt") ) == NULL){ // 读写方式打开目标
		printf("can't ope the out file!\n");
		exit(0);
	}
	if( (hit_rate= fopen(argv[3],"a") ) == NULL){
			printf("can't ope the hit rate file!\n");
			exit(0);
	}
	LRUcache mycache;
	while(fscanf(in,"%X",&addr)!=EOF)  
	{  
		//printf("%X \t",val);//输出到屏幕  
		int rd= mycache.access_cache( addr ); // 每读取一次，模拟放入缓存，计算RD
		int setNo=( (addr -0x80000000 )/128 )% SET_NUM;
		unsigned int merge_addr=( addr>>7 )<< 7;
		fprintf(out,"%X\t%X\t%d\t%d\n",addr,merge_addr,setNo,rd); // RD保存到 out 文件中

	}  
	fprintf(hit_rate,"%lld  %lld  %f\n ", mycache.hit_num, mycache.miss_num, 
					mycache.hit_num* 1.0 /(mycache.hit_num + mycache.miss_num) );
	fclose(in);  
	fclose(out);
	fclose(hit_rate);
}

