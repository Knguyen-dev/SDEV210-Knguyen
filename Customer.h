#ifndef Customer_H
#define Customer_H
#include <string>
#include <ostream>

class Customer {
private: 
	int customer_id;
	std::string fname;
	std::string lname;
	std::string email;
	int points;
public:

	Customer() : customer_id(0), fname(""), lname(""), email(""), points(0) {}

	Customer(int customer_id, std::string fname, std::string lname, std::string email, int points) 
		: customer_id(customer_id), fname(fname), lname(lname), email(email), points(points) {}
	
	const int getCustomerID() {
		return customer_id;
	}

	const std::string& getFirstName() {
		return fname;
	}

	void setFirstName(const std::string& newFirstName) {
		fname = newFirstName;
	}

	const std::string& getLastName() {
		return lname;
	}

	void setLastName(const std::string& newLastName) {
		lname = newLastName;
	}

	const std::string& getEmail() {
		return email;
	}

	void setEmail(const std::string& newEmail) {
		email = newEmail;
	}

	const int getPoints() {
		return points;
	}

	void setPoints(const int newPoints) {
		points = newPoints;
	}

	friend std::ostream& operator<<(std::ostream& os, const Customer& customer) {
		os << "<Customer ID(" << customer.customer_id << "), fname(" << customer.fname << "), lname(" << customer.lname << "), email(" << customer.email << "), points(" << customer.points << ")/>";
		return os;
	}

	bool operator!() const {
		return customer_id == 0;
	}
};

#endif