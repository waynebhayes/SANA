#ifndef CLUSTERMODE_HPP_
#define CLUSTERMODE_HPP_

#include <vector>
#include <string>

#include "Mode.hpp"
#include "../arguments/ArgumentParser.hpp"

using namespace std;
class ClusterMode : public Mode {
public:
	void run(ArgumentParser& args);
	void submitToCluster(const vector<string>& argv);
	std::string getName(void);
private:
	string makeScript(const vector<string>& argv);
	string getQModeArgValue(const vector<string>& argv);
	uint getOArgValueIndex(const vector<string>& argv);
};

#endif /* CLUSTERMODE_HPP_ */
