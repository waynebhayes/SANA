/* format.h
Author: Jialu Hu
Data: 10.12.2012*/
#ifndef FORMAT_H_
#define FORMAT_H_
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unordered_map>

template<typename NetworksType,typename MyOption>
class Format
{
public:
  Format(MyOption&);
  ~Format(){};
  std::vector<std::string> blastfile;
  std::unordered_multimap<std::string,std::string>  KOmap;
  std::unordered_map<std::string,unsigned> KOgroup;
	float mEntropy,mNormalizedEntropy;
  bool removeBiEdges(NetworksType&);
  bool formatAlignment(std::string,std::string);
  bool removeRodundantInteraction();
  bool retrieveKOnumber(std::string);
  bool extractHomologyProteins(std::string,NetworksType&);
  bool extractGoAssociation(std::string);
  bool extractGraemlinAlignment(std::string,std::string);
  bool readTrainingData(std::string);
  bool retrieveKOgroups(std::string);
	void calculateEntropy(std::unordered_map<int,int>&,std::vector<std::string>&);
	void retrieveGIlist();
};

template<typename NetworksType,typename MyOption>
Format<NetworksType,MyOption>::Format(MyOption& myoption)
{
	mEntropy=0.0;
	mNormalizedEntropy=0.0;
  blastfile = myoption.blastfiles;
}

template<typename NetworksType,typename MyOption>
void Format<NetworksType,MyOption>::retrieveGIlist()
{
	std::vector<std::string> filelist;
	filelist.push_back("./dataset/graemlin/0-coli.tab");
	filelist.push_back("./dataset/graemlin/1-cholerae.tab");
	filelist.push_back("./dataset/graemlin/2-campy.tab");
	filelist.push_back("./dataset/graemlin/3-pylori.tab");
	filelist.push_back("./dataset/graemlin/4-caulo.tab");
	filelist.push_back("./dataset/graemlin/5-salmo.tab");
	std::ifstream input;
	std::string line,gene1,gene2,filename;
	std::unordered_map<std::string, int> genesmap;
	for(unsigned i=0;i<filelist.size();i++)
	{
		input.open(filelist[i].c_str());
		std::getline(input,line);// header line interactor A and B
		while(std::getline(input,line))
		{
			std::stringstream streamline(line);
			streamline >> gene1 >> gene2;
			if(genesmap.find(gene1)==genesmap.end())
				genesmap[gene1]=1;
			if(genesmap.find(gene2)==genesmap.end())
				genesmap[gene2]=1;
		}
		input.close();
	}
	unsigned num=0,pos=0;
	filename.append("./dataset/graemlin/GiList-");
	filename.append(convert_num2str(pos));
	filename.append(".txt");
	std::ofstream output(filename);
	for(std::unordered_map<std::string,int>::iterator it=genesmap.begin();it!=genesmap.end();++it,++num)
	{
		if(num>=2000)
		{
			num=0;
			pos++;
			output.close();
			filename.clear();
			filename.append("./dataset/graemlin/GiList-");
			filename.append(convert_num2str(pos));
			filename.append(".txt");
			output.open(filename.c_str());
		}
		output << it->first << std::endl;
	}
	output.close();
}

template<typename NetworksType,typename MyOption>
bool Format<NetworksType,MyOption>::removeBiEdges(NetworksType& networks)
{
  std::vector<std::string>::iterator it;
  for(it=blastfile.begin();it!=blastfile.end();++it)
  {
    std::string filename = *it;
    std::string outname=filename;
    outname.append(".sw");
    std::ifstream input(filename);
    std::ofstream output(outname);
    if(!input.is_open() && !output.is_open())
      std::cout << "Error in opening the files" << std::endl;
    std::string line="";
    while(std::getline(input,line))
    {
      std::string protein1,protein2;
      std::stringstream streamline(line);
      streamline >> protein1 >> protein2;
      if(networks.existNode(protein1) && networks.existNode(protein2))
      {
        output << line <<std::endl;
      }
    }
  }
  return true;
}

template<typename NetworksType,typename MyOption>
bool Format<NetworksType,MyOption>::formatAlignment(std::string alignmentfile,std::string outfile)
{
  std::ifstream input(alignmentfile.c_str());
  std::ofstream output(outfile.c_str());
  std::string line;
  while(std::getline(input,line))
  {
    unsigned i=0;
    std::stringstream streamline(line);
    std::vector<std::string> proteins;
    while(streamline.good())
    {
      std::string protein;
      streamline >> protein;
      if(protein.compare("NA")!=0)
      proteins.push_back(protein);
    }
    if(proteins.size()>=2)
    {
      for(i=0;i<proteins.size()-1;i++)
      {
        output << proteins[i] << "\t";
      }
      output << proteins[i] << std::endl;
    }
  }
  return true;	
}

template<typename NetworksType,typename MyOption>
bool Format<NetworksType,MyOption>::removeRodundantInteraction()
{
	return true;
}

template<typename NetworksType,typename MyOption>
bool Format<NetworksType,MyOption>::readTrainingData(std::string formatfile)
{
	std::ifstream input(formatfile.c_str());
	std::string line;
	unsigned index=1;
	while(std::getline(input,line))
	{
		std::stringstream streamline(line);
		while(streamline.good())
		{
			std::string term;
			streamline >> term;
			KOgroup[term] = index;
		}
		index++;
	}
	input.close();
	return true;
}

template<typename NetworksType,typename MyOption>
bool Format<NetworksType,MyOption>::retrieveKOgroups(std::string formatfile)
{
	std::ifstream input(formatfile.c_str());
	std::string line;
	std::string trainingfile("./benchmark/graemlin/graemlin-2.0_test_files/test_cases/6way/training.txt");
	unsigned linenum=0;

	readTrainingData(trainingfile.c_str());
	std::vector<std::string> matchset;
	std::unordered_map<int,int> KO_Id;
	while(std::getline(input,line))
	{
		matchset.clear();
		std::stringstream streamline(line);
		while(streamline.good())
		{
			std::string term;
			streamline >> term;
			matchset.push_back(term);
			//if(KOgroup.find(term)==KOgroup.end())continue;
			//unsigned myindex=KOgroup[term];//discard all nodes in the alignment without a KO group
			//matchsetSize++;
			//if(indexMap.find(myindex)!=indexMap.end())
			//{
			//	indexMap[myindex]++;
			//}else
			//{
			//	indexMap[myindex]=1;
			//}
			//if(indexMap[myindex]>maxKOnum) maxKOnum=indexMap[myindex];
		}
		calculateEntropy(KO_Id,matchset);
		linenum++;
	}
	mEntropy/=linenum;
	mNormalizedEntropy/=linenum;
	std::cout <<"#The mean entropy and mean normalized entropy in  "<<formatfile <<" are "<< mEntropy<<"\t"<<mNormalizedEntropy<<std::endl;
	return true;
}

template<typename NetworksType,typename MyOption>
void Format<NetworksType,MyOption>::calculateEntropy(std::unordered_map<int,int>& KO_Id,std::vector<std::string>& matchset)
{
	unsigned mysize=matchset.size();
	std::string ele;
	int id;
	KO_Id.clear();
	float entropy=0.0;
	for(unsigned i=0;i<mysize;i++)
	{
		ele=matchset[i];
		if(KOgroup.find(ele)==KOgroup.end())continue;
		id=KOgroup[ele];
		if(KO_Id.find(id)==KO_Id.end())
			KO_Id[id]=1;
		else
			KO_Id[id]++;		
	}
	int d=0;
	for(std::unordered_map<int,int>::iterator it=KO_Id.begin();it!=KO_Id.end();++it)
	{
		float p_i=(1.0*it->second)/mysize;
		entropy+=(-1.0)*p_i*log(p_i);
		d++;
	}
	mEntropy+=entropy;
	if(d>1)	mNormalizedEntropy+=(entropy/log(d));
}

template<typename NetworksType,typename MyOption>
bool Format<NetworksType,MyOption>::retrieveKOnumber(std::string formatfile)
{
	std::ifstream input(formatfile.c_str());
	std::string line;
	std::string module;
	std::string KOnumber;
	while(std::getline(input,line))
	{
	  std::stringstream streamline(line);
	  std::string koindex;
      streamline >> koindex;
      if(koindex.compare("D")==0)
      {
		  streamline >> module;
	  }else if(koindex.compare("E")==0)
	  {
		  streamline >> KOnumber;
		  std::cout << KOnumber << std::endl;
		  KOmap.insert(std::make_pair(module,KOnumber));
	  }
	  else continue;
	}
	return true;
}

template<typename NetworksType,typename MyOption>
bool Format<NetworksType,MyOption>::extractHomologyProteins(std::string filename,NetworksType& networks)
{
	std::ifstream input(filename.c_str());
	std::string inputNet[]={"Celeg20130131","Dmela20130131","Ecoli20130131","Hpylo20130131","Hsapi20130131","Mmusc20130131","Rnorv20130131","Scere20130131"};
	std::string line;
	std::getline(input,line);// Header line
	std::unordered_map<std::string,int> checklist;
	while(std::getline(input,line))
	{
		std::stringstream streamline(line);
		std::string protein1,protein2;
		double evalue;
		streamline >> protein1 >> protein2 >> evalue;
		unsigned i=networks.getHost(protein1);
		unsigned j=networks.getHost(protein2);
		if(i==100 || j==100) continue;
		if(i>j)
		{
			unsigned temp=i;
			std::string protein=protein1;
			i=j;
			j=temp;
			protein1=protein2;
			protein2=protein;
		}
		if(checklist.find(protein1)==checklist.end())
		{
		  checklist[protein1]=1;
		  std::cout << protein1 <<std::endl;
	    }
		if(checklist.find(protein2)==checklist.end())
		{
		  checklist[protein2]=1;
		  std::cout << protein2 <<std::endl;
	    }
		std::string outfilename("./dataset/dip/");
    outfilename.append(inputNet[i]);
		outfilename.append("-");
		outfilename.append(inputNet[j]);
		outfilename.append(".evals");
		std::ofstream output(outfilename.c_str(),std::ios_base::out|std::ios_base::app);
		output << protein1 <<"\t" << protein2 << "\t" << evalue << std::endl;
		output.close();		
	}
	return true;
}

template<typename NetworksType,typename MyOption>
bool Format<NetworksType,MyOption>::extractGoAssociation(std::string formatfile)
{
	std::ifstream input(formatfile);
	std::ofstream output("./dataset/goa/gene_association.goa_target");
	std::unordered_map<std::string,int> checklist;
	std::string line;
	while(std::getline(input,line))
	{
		std::stringstream streamline(line);
		std::string gi_id,uni_id;
		streamline >> gi_id >> uni_id;
		if(gi_id.compare("From")==0)continue;
		if(checklist.find(uni_id)==checklist.end())
		{
		  checklist[uni_id]=1;
	    }				
	}
	input.close();
	std::ifstream input2("./dataset/goa/gene_association.goa_uniprot");
	if(!input2.is_open())
	{
		std::cerr << " Gene association file can't open." << std::endl;
	}
	while(std::getline(input2,line))
	{
		if(line[0]=='!')continue;
		std::vector<std::string> goterm;
		std::string newline=line;
		std::size_t found=line.find_first_of("\t");
		while(found!=std::string::npos)
		{
			std::string term;
			term=line.substr(0,found);
			goterm.push_back(term);
			line=line.substr(found+1);
			found=line.find_first_of("\t");
		}
		if(checklist.find(goterm[1])!=checklist.end() &&
			goterm[6].compare("IEA")!=0 &&
			goterm[6].compare("ISS")!=0)
			output << newline << std::endl;
		 //output << goterm[1] << "\t" << goterm[3] << "\t" << goterm[5] << "\t" << goterm[7] << std::endl;
	}
	input2.close();
	output.close();
	return true;
}

template<typename NetworksType,typename MyOption>
bool Format<NetworksType,MyOption>::extractGraemlinAlignment(std::string formatfile,std::string outfile)
{
	std::ifstream input(formatfile.c_str());
	std::ifstream input2("./dataset/graemlin/gi-map.txt");
	std::ofstream output(outfile.c_str());
	std::string line;
	std::unordered_map<std::string,std::string> idmap;
	
	while(std::getline(input2,line))
	{
		std::stringstream streamline(line);
		std::string id1,id2;
		streamline >> id1 >> id2;
		if(id1.compare("From")==0)continue;
		idmap[id1]=id2;
	}

    /// The next line goes for header line. If there is no header line, set it as comment.
	//std::getline(input, line);
	while(std::getline(input,line))
	{
		std::vector<std::string> matchset;
		std::stringstream streamline(line);
		while(streamline.good())
		{
			std::string term;
			streamline >> term;
			matchset.push_back(term);
		}
		if(matchset.size()<2)continue;
		unsigned i=0;
		for(i=0;i<matchset.size()-1;i++)
		{
		   std::string uniprot_id;
		   if(idmap.find(matchset[i])!=idmap.end())
		   {
			  uniprot_id=idmap[matchset[i]];
			  output << uniprot_id <<"\t";
		   }else
		   {
			   output << matchset[i] <<"\t";
		   }
	   }
		if(idmap.find(matchset[i])!=idmap.end())
		{
		  output << idmap[matchset[i]] <<std::endl;
	   }else
	   {
		   output << matchset[i] <<std::endl;
	   }
	}
	return true;
}
#endif
