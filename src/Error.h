#pragma once

/*
Project: OrgMaker 3

File: src/Error.h
Purpose: Declare the error class

Authors: Regan "cuckydev" Green
*/

//Standard library
#include <iostream>
#include <vector>
#include <string>

//Error class
class Error
{
	private:
		//Error list
		std::vector<std::string> error_list;
		
	public:
		//Add error to error list
		bool Push(const Error &error) { error_list.push_back(error.ToString()); return true; }
		bool Push(const std::string &error) { error_list.push_back(error); return true; }
		
		//Clear all errors in the error list
		void Clear() { error_list.clear(); }
		
		//Return string that contains all errors
		std::string ToString() const;
		
		//Operators
		friend std::ostream& operator<<(std::ostream &out, const Error &x) { out << x.ToString(); return out; }
		operator bool() const { return error_list.size() > 0; }
		operator std::string() const { return ToString(); }
};
