#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <fstream>
#include <map>

using namespace std;

template<class T>
string to_string(const T& t){
	 ostringstream oss;
	 oss<<t;
	 return oss.str();
}
class Compress {
	public:
		Compress(string loadname,string outname,bool flag){
			this->loadname=loadname;
			this->outname=outname;
			if(!flag)
				todo();
			else
				undo();
		}
		void todo(){
			unsigned char c;
			char cc[1];
			int i;
			int len;
			string rc;
			string preout("");
			string out("");
			ifstream loadfile(loadname.data(),ios::in);//只读不创建
			if(!loadfile){
				cout<<"file load error";
				return;
			}
			for(i=0;i<128;i++){
				ascii_count[i]=0;
			}
			while(!loadfile.eof()){
				loadfile.read(cc,1);
				ascii_count[cc[0]]++;
			}
			creat_huffcode();//得到哈夫曼编码hash;
			loadfile.clear();
			loadfile.seekg(0);
			
			while(!loadfile.eof()){
				loadfile.read(cc,1);
				preout+=hash[to_string(cc[0])];
			}
			loadfile.close();
			len=preout.length();
			for(i=0;i<8-len;i++){
				preout+="0";
			}
			c='\0';
			for(i=0;i<preout.length();i++){
				if(i%8==0&&i!=0){
					out+=to_string(c);
					c='\0';
				}
				c+=(preout[i]-'0');
				if(i%8!=7)
					c<<=1;
			}
			out+=to_string(c);
			map<string,string>::iterator itr;
			string huffcode="";
			for(itr=hash.begin();itr!=hash.end();itr++)//记录编码信息
				huffcode+=("%"+itr->first+""+itr->second);
			ofstream outfile(outname.data());
			outfile<<"jerehao-compress##"<<huffcode<<"##"<<out;
			outfile.close();
		}
		void undo(){
			char cc[2];//cc[1]记录前一个值
			int key_count=0;//标记编码信息头尾
			string huff_key="";
			string huff_code="";
			string temp="";
			string preout="";
			string out="";
			int a;//转2进制字符串中间变量
			int i,len;

			ifstream loadfile(loadname.data(),ios::in);
			if(!loadfile){
				cout<<"file load error";
				return;
			}
			while(!loadfile.eof()){
				loadfile.read(cc,1);
				if(key_count<2){
					if(cc[0]==cc[1]&&cc[1]=='#')
						key_count++;
					if(key_count==1){
						if(cc[1]=='%'){
							if(huff_key.size()>0)
								hash[huff_code.erase(huff_code.size()-1,1)]=huff_key;
							huff_key=to_string(cc[0]);
							huff_code="";
						}
						else{
							huff_code=huff_code+to_string(cc[0]);
						}
					}
					else if(key_count==2){
						hash[huff_code.erase(huff_code.size()-1,1)]=huff_key;
					}
				}
				else{//读到正文
					a=(int)cc[0];
					if(a<0)
						a=a*(-1)+128;
					temp="";
					while(a>0){
						temp=to_string(a%2)+temp;
						a=a/2;
					}
					len=temp.size();
					for(i=0;i<8-len;i++)
						temp="0"+temp;
					preout+=temp;
				}
				cc[1]=cc[0];
				
			}
			if(key_count<2)
				return;
			a=1;
			len=preout.size();
			for(i=0;i<len;i++){
				temp=preout.substr(0,a);
				if(hash.find(temp)!=hash.end()){
					out+=hash[temp];
					preout.erase(0,a);
					a=1;
				}
				else
					a++;
			}
		}

	private:
		int ascii_count[128];
		map<string,string> hash;//编码解码通用
		map<string,int> huff_hash;
		string loadname;
		string outname;
		void creat_huffcode(){
			int i,j,temp;
			char c;
			string s;
			for(i=0;i<128;i++){
				if(ascii_count[i]>0){
					c=i;
					stringstream stream;
					stream << c;
					huff_hash[stream.str()]=ascii_count[i];
				}
			}
			while(huff_hash.size()>1){
				select_2min_and_combine();
			}
		}
		void select_2min_and_combine(){
			int i;
			int min;
			string s;
			string min1,min2;
			if(huff_hash.size()<2)
				return;
			map<string,int>::iterator itr;
			itr=huff_hash.begin();

			min1=itr->first;
			min2=(++itr)->first;
			if(huff_hash[min2]<huff_hash[min1]){
				min2=min1;
				min1=itr->first;
			}
			for(itr++;itr!=huff_hash.end();itr++){
				if(itr->second<huff_hash[min1]){
					min2=min1;
					min1=itr->first;
				}
				else if(itr->second<huff_hash[min2]){
					min2=itr->first;
				}
			}
			for(i=0;i<min1.size();i++){
				s=min1[i];
				if(hash.find(s)!=hash.end())
					hash[s]="0"+hash[s];
				else
					hash[s]="0";
			}
			for(i=0;i<min2.size();i++){
				s=min2[i];
				if(hash.find(s)!=hash.end())
					hash[s]="1"+hash[s];
				else
					hash[s]="1";

			}
			min=huff_hash[min1]+huff_hash[min2];
			huff_hash.erase(min1);
			huff_hash.erase(min2);
			huff_hash[min1+min2]=min;
			
		}
};

int main(int argc,char **argv){//命令形式如 [-d] -l loadname -o outname
	string loadname;
	string outname;
	bool is_d=false;
	int i;
	for(i=1;i<argc;i++){
		if(strcmp(argv[i],"-d")==0)
			is_d=true;
		else if(strcmp(argv[i],"-l")==0)
			loadname=argv[i+1];
		else if(strcmp(argv[i],"-o")==0)
			outname=argv[i+1];
		else if(strcmp(argv[i],"-h")==0){
			cout<<"parameter should be [-d] -l loadname -o outname";
			return 0;
		}

	}
	if(loadname.length()<1||outname.length()<1){
		cout<<"parameter error,parameter should be [-d] -l loadname -o outname";
		return 1;
	}
	Compress cp(loadname,outname,is_d);

	return 0;
}
