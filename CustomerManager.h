#ifndef CustomerManager_H
#define CustomerManager_H
#include <string>
#include <vector>
#include "DBConn.h"
#include "Customer.h"


/*
+ CustomerManager: Class that encapsulates and handles operations with the 'Customer' table
*/

class CustomerManager {
private:
	DBConn& dbConn;
	std::string tableName;

	/*
	- Constants that define the maximum lengths of the customers table.
	
	NOTE: If you make changes to the length constraints, to see these changes, delete 
	the current customers table and re-initialize it.
	*/
	static const int MAX_FNAME_LENGTH = 50;
	static const int MAX_LNAME_LENGTH = 50;
	static const int MAX_EMAIL_LENGTH = 50;
	
	// Helper function that takes SQL row data and creates customer object from it.
	Customer createCustomerFromRow(SQLINTEGER customer_id, SQLCHAR* fname, SQLCHAR* lname, SQLCHAR* email, SQLINTEGER points) {
		// Null terminate the strings
		fname[MAX_FNAME_LENGTH] = '\0';
		lname[MAX_LNAME_LENGTH] = '\0';
		email[MAX_EMAIL_LENGTH] = '\0';

		// Convert SQLCHAR to strings and SQLINTEGER to int
		std::string fStr(reinterpret_cast<char*>(fname));
		std::string lStr(reinterpret_cast<char*>(lname));
		std::string eStr(reinterpret_cast<char*>(email));
		int intID = static_cast<int>(customer_id);
		int intPoints = static_cast<int>(points);

		// Create and return customer's object representation
		Customer customer(intID, fStr, lStr, eStr, intPoints);
		return customer;
	}

public:
	CustomerManager(DBConn& dbConn, std::string tableName) : dbConn(dbConn), tableName(tableName) {}

	void initTable() {
		std::string query = "CREATE TABLE " + tableName + " ( "
			"customer_id INT NOT NULL IDENTITY PRIMARY KEY, "
			"fname VARCHAR(" + std::to_string(MAX_FNAME_LENGTH) + ") NOT NULL, "
			"lname VARCHAR(" + std::to_string(MAX_LNAME_LENGTH) + ") NOT NULL, "
			"email VARCHAR(" + std::to_string(MAX_EMAIL_LENGTH) + ") NOT NULL, "
			"points INT NOT NULL "
			");";
		if (!dbConn.executeSQL(query)) {
			throw std::runtime_error("Failed to initialize '" + tableName + "' table!");
		}
	}

	/*
	- Ensures the string attributes that were inputted meet max length constraints.
	- If they aren't then an error is thrown
	*/
	void validateFirstName(std::string fname) {
		if (fname.length() > MAX_FNAME_LENGTH) {
			throw std::runtime_error("Customer fname exceeds maximum length of " + std::to_string(MAX_FNAME_LENGTH) + " characters!");
		}
	}

	void validateLastName(std::string lname) {
		if (lname.length() > MAX_LNAME_LENGTH) {
			throw std::runtime_error("Customer lname exceeds maximum length of " + std::to_string(MAX_LNAME_LENGTH) + " characters!");
		}
	}

	void validateEmail(std::string email) {
		if (email.length() > MAX_EMAIL_LENGTH) {
			throw std::runtime_error("Customer email exceeds maximum length of " + std::to_string(MAX_EMAIL_LENGTH) + " characters!");
		}
	}

	std::vector<Customer> fetchCustomers(const std::string query) {
		std::vector<Customer> customers;

		// Execute query to fetch customers
		if (!dbConn.executeSQL(query)) {
			throw std::runtime_error("Failed to query customers!");
		}

		// Create buffers/variables
		SQLINTEGER customer_id = 0;
		SQLCHAR fname[MAX_FNAME_LENGTH + 1] = {};
		SQLCHAR lname[MAX_LNAME_LENGTH + 1] = {};
		SQLCHAR email[MAX_EMAIL_LENGTH + 1] = {};
		SQLINTEGER points = 0;

		// Bind columns
		dbConn.bindColumn(1, SQL_INTEGER, &customer_id, sizeof(customer_id));
		dbConn.bindColumn(2, SQL_C_CHAR, fname, sizeof(fname));
		dbConn.bindColumn(3, SQL_C_CHAR, lname, sizeof(lname));
		dbConn.bindColumn(4, SQL_C_CHAR, email, sizeof(email));
		dbConn.bindColumn(5, SQL_INTEGER, &points, sizeof(points));

		// Fetch rows and store them in the vector
		while (true) {
			// Fetch the row
			SQLRETURN retcode = dbConn.fetchRow();
			if (retcode == SQL_NO_DATA) {
				// No more rows to fetch, exit the loop
				break;
			}
			else if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
				dbConn.closeCursor(); // Close the cursor before the error was thrown.
				throw std::runtime_error("Failed to fetch a given customer!");
			}

			// Create customer object from row data, and push it into the vector
			Customer customer = createCustomerFromRow(customer_id, fname, lname, email, points);
			customers.push_back(customer);
		}

		/*
		- Close the cursor. When fetching from a result set (a table), the database driver uses a cursor to keep
		track of your position in teh result set (what row we're currently looking at). Closing the cursor releases
		its resources and frees up memory. It's important to close the cursor when you're done fetching
		rows to ensure proper resource management, avoid memory leaks, and unexpected errors with SQL
		*/
		dbConn.closeCursor();

		// Return the vector of customers
		return customers;
	}

	// Returns a vector of all customers in our database
	std::vector<Customer> getAllCustomers() {
		// Execute query to fetch customers
		std::string query = "SELECT * FROM " + tableName + ";";
		std::vector<Customer> customers = fetchCustomers(query);
		return customers;
	}

	// Returns a customer by their customer_id
	Customer getCustomerByID(int customer_id) {
		std::string query = "SELECT * FROM " + tableName + " WHERE customer_id=" + std::to_string(customer_id) + ";";

		std::vector<Customer> customers = fetchCustomers(query);
		if (customers.empty()) {
			throw std::runtime_error("Customer with ID '" + std::to_string(customer_id) + "' wasn't found!");
		}

		// Expecting a vector with only one customer
		Customer customer = customers[0];
		
		// Execute SQL Statement
		return customer;
	}

	// Creates a customer and returns that customer 
	Customer createCustomer(std::string fname, std::string lname, std::string email, int points) {

		// Ensure that the input meets input length constraints before checking with the database.
		validateFirstName(fname);
		validateLastName(lname);
		validateEmail(email);

		// Escape the input of potential single quotes; use this in the SQL query
		std::string escaped_fname = dbConn.escapeSQL(fname);
		std::string escaped_lname = dbConn.escapeSQL(lname);
		std::string escaped_email = dbConn.escapeSQL(email);

		// Construct and execute SQL query
		std::string query = "INSERT INTO " + tableName + " (fname, lname, email, points) VALUES ('" + escaped_fname + "', '" + escaped_lname + "', '" + escaped_email + "', " + std::to_string(points) + ")";
		if (!dbConn.executeSQL(query)) {
			throw std::runtime_error("Failed to create customer '" + fname + " " + lname + "' with email '" + email + "'!");
		}

		// Create and return a Customer object, use original unescaped input
		const int id = dbConn.getLastInsertedID();
		Customer customer(id, fname, lname, email, points);
		return customer;
	}

	// Updates fname column of row with customer_id
	void updateFirstName(int customer_id, std::string fname) {
		
		// Validate length of first name
		validateFirstName(fname);

		// escape it
		fname = dbConn.escapeSQL(fname);

		// Execute sql query
		std::string query = "UPDATE " + tableName + " SET fname='" + fname + "' WHERE customer_id=" + std::to_string(customer_id) + ";";
		if (!dbConn.executeSQL(query)) {
			throw std::runtime_error("Failed to update customer with id '" + std::to_string(customer_id) + "'!");
		}
	}

	// Updates lname column of row with customer_id
	void updateLastName(int customer_id, std::string lname) {

		// Validate length of last name
		validateLastName(lname);

		// Escape the last name
		lname = dbConn.escapeSQL(lname);

		// Execute SQL Query
		std::string query = "UPDATE " + tableName + " SET lname='" + lname + "' WHERE customer_id=" + std::to_string(customer_id) + ";";
		if (!dbConn.executeSQL(query)) {
			throw std::runtime_error("Failed to update customer with id '" + std::to_string(customer_id) + "'!");
		}
	}

	// Updates email column of row with customer_id
	void updateEmail(int customer_id, std::string email) {
		// Validate length of email
		validateEmail(email);

		// Escape the email
		email = dbConn.escapeSQL(email);

		// Execute SQL Query
		std::string query = "UPDATE " + tableName + " SET email='" + email + "' WHERE customer_id=" + std::to_string(customer_id) + ";";
		if (!dbConn.executeSQL(query)) {
			throw std::runtime_error("Failed to update customer with id '" + std::to_string(customer_id) + "'!");
		}
	}

	void updatePoints(int customer_id, int points) {
		std::string query = "UPDATE " + tableName + " SET points='" + std::to_string(points) + "' WHERE customer_id=" + std::to_string(customer_id) + ";";
		if (!dbConn.executeSQL(query)) {
			throw std::runtime_error("Failed to update customer points with id '" + std::to_string(customer_id) + "'!");
		}
	}

	// Deletes customer with customer_id from table
	void deleteCustomer(int customer_id) {
		std::string query = "DELETE FROM " + tableName + " WHERE customer_id=" + std::to_string(customer_id) + ";";

		if (!dbConn.executeSQL(query)) {
			throw std::runtime_error("Failed to delete customer with id '" + std::to_string(customer_id) + "'. Customer may not exist!");
		}		
	}
};


#endif