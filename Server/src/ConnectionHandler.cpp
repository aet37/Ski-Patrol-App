/**
 * @file ConnectionHandler.cpp
 * 
 * @brief Implementation of the ConnectionHandler class
 */

// SYSTEM INCLUDES
#include <iostream>
#include <boost/bind.hpp>
#include <string>
#include <vector>

// C++ PROJECT INCLUDES
#include "ConnectionHandler.hpp" // Header for class
#include "Request.hpp" // For Common::Request
#include "Response.hpp" // For Common::Response

std::string slopes[] = {"--","Other (Explain Below)", "Alpine Meadows", "Avalanche", "Beginner Area", "Blitzen Chairline", "Boomerang", "Cortina Trail", "Deer Pass Trail", "Fawn Lane", "Giant Boulder", "Giant Steps", "Goosebumps", "Great Western", "Gunnar", "Gunnar Chairline", "Little Boulder", "Little North Face", "Lost Boy", "Lost Girl", "North Face", "Phillip's Run", "Stowe", "Stowe Beginner Bowl", "Stowe Trail", "Tyrol",  "Village Trail", "Wagner", "Yodeler", "Yodeler Bypass"};

void ConnectionHandler::Start()
{
    m_socket.async_read_some(boost::asio::buffer(m_data, ConnectionHandler::MAX_LENGTH),
                             boost::bind(&ConnectionHandler::HandleRead,
                             shared_from_this(),
                             boost::asio::placeholders::error,
                             boost::asio::placeholders::bytes_transferred));
}

void ConnectionHandler::HandleRead(const boost::system::error_code& rErr, size_t bytesTransferred)
{
    if (rErr)
    {
    	if(rErr.message() != "End of file")
    	{
		    std::cerr << "error: " << rErr.message() << std::endl;
	    }
        m_socket.close();
        return;
    }

    // Terminate what was transferred
    m_data[bytesTransferred] = '\0';

    // Just print out the received data if it is not a connection request
    //std::cout << "Server received " << m_data << std::endl;

    // Parse the data into the request structure
    Common::Request req;
    ParseRequest(req);

    // Determine what needs done for this request
    HandleRequest(req);

	m_socket.async_write_some(boost::asio::buffer(m_message, ConnectionHandler::MAX_LENGTH),
	                          boost::bind(&ConnectionHandler::HandleWrite,
	                                      shared_from_this(),
	                                      boost::asio::placeholders::error,
	                                      boost::asio::placeholders::bytes_transferred));
}

void ConnectionHandler::HandleWrite(const boost::system::error_code& rErr, size_t bytesTransferred)
{
	// Print out reply if it is not an error or a connection check reply
    if (!rErr)
    {
        // Just print out a message
        //std::cout << "Server sent " << m_message.c_str() << std::endl;
    }
    else
    {
    	// Don't print End of file errors
	    if(rErr.message() != "End of file")
	    {
		    std::cerr << "error: " << rErr.message() << std::endl;
	    }
        m_socket.close();
    }
}

void ConnectionHandler::ParseRequest(Common::Request& rReq)
{
    try
    {
        // Convert data to an string
        std::string data = std::string(m_data);

        std::string requestCode = "";
        std::string additionalData = "";
        if (data.find_first_of(" ") == std::string::npos)
        {
            requestCode = data;
        }
        else
        {
            requestCode = data.substr(0, data.find_first_of(" "));
            additionalData = data.substr(data.find_first_of(" ") + 1);
        }

        // First characters should be the request code
        rReq.SetRequestCode(static_cast<Common::RequestCode>(std::stoi(requestCode)));

        // Place the additional data into the data field of the request
        rReq.SetData(additionalData);
    }
    catch (std::exception& e)
    {
        std::cout << "Invalid command " << m_data << std::endl;
        rReq.SetRequestCode(Common::RequestCode::ERROR);
    }
}

void ConnectionHandler::HandleRequest(Common::Request& rReq)
{
    Common::Response resp;

    // Return early if the request code is ERROR
    if (rReq.GetRequestCode() == Common::RequestCode::ERROR)
    {
        resp.SetResponseCode(Common::ResponseCode::ERROR);
        m_message = resp.ToString();
        return;
    }

     switch (rReq.GetRequestCode())
     {
     	case Common::RequestCode::CONNECTION_CHECK:
        {
        	resp.SetResponseCode(Common::ResponseCode::SUCCESS);
        	break;
        }
         case Common::RequestCode::REPORT:
         {
         	// Parse the data sent
         	int slopeListNum = std::stoi(rReq.GetData().substr(0, 2));
         	std::string location = rReq.GetData().substr(9, rReq.GetData().size() - 9);

        	// Print Accident to console
        	std::cout << std::endl << "!! ACCIDENT REPORTED !!" << std::endl;
        	std::cout << "Slope: " << slopes[slopeListNum] << std::endl;
        	std::cout << "\t" << location << std::endl;
        	if(rReq.GetData()[3] == '1')
	        {
        		std::cout << "\tBlood Reported" << std::endl;
	        }
			 if(rReq.GetData()[5] == '1')
			 {
			     std::cout << "\tNot Moving" << std::endl;
			 }
	         if(rReq.GetData()[7] == '1')
	         {
		         std::cout << "\tMultiple Victims" << std::endl;
	         }

        	// Send sucess to client
        	resp.SetResponseCode(Common::ResponseCode::SUCCESS);

        	// Print acknowlegdement to screen
        	std::cout << "Server Acknowledges" << std::endl;

            break;
        }
         default:
             std::cout << "Invalid RequestCode " << static_cast<int>(rReq.GetRequestCode()) << std::endl;
             resp.SetResponseCode(Common::ResponseCode::ERROR);
             break;
     }

    // Set the message, so the requester will receive the response
    m_message = resp.ToString();
}
