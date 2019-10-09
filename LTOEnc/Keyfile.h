#pragma once

class Keyinfo
{
public:
	char* key;
	int keySize;
	bool valid;
	std::string check;
	void load(std::string hexinput);
	Keyinfo();
	~Keyinfo();
private:
	void loadKey(std::string str);
	void loadCheck();

};
