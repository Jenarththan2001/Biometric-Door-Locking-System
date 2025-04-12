import React, { useState } from "react";
import 'bootstrap/dist/css/bootstrap.min.css';
import '../style.css';

const AttendanceSystem = () => {
  const [attendanceData, setAttendanceData] = useState([]);

  // Example of adding an attendance entry (this would be dynamically updated in a real application)
  const addAttendance = (name, nic, age, phoneNumber, id) => {
    const newEntry = {
      name,
      nic,
      age,
      phoneNumber,
      id,
      attendanceTime: new Date().toLocaleTimeString(),
      attendanceDate: new Date().toLocaleDateString(),
    };
    
    setAttendanceData([...attendanceData, newEntry]);
  };

  // For demo purposes, you can simulate adding attendance by calling this function
  const handleAddAttendance = () => {
    const attendees = [
      ["John Doe", "123456789V", "25", "0771234567", "01"],
      ["Tom Jerry", "123456456V", "27", "0771234560", "02"],
      ["Alice Smith", "987654321V", "22", "0779876543", "03"],
      ["Bob Brown", "456123789V", "30", "0776543219", "04"],
      ["Charlie Green", "741852963V", "28", "0773692581", "05"],
      ["Diana Prince", "159357486V", "26", "0771593574", "06"]
    ];
  
    // Pick a random attendee from the list
    const randomIndex = Math.floor(Math.random() * attendees.length);
    const [name, nic, age, phone, id] = attendees[randomIndex];
  
    // Call addAttendance with the randomly selected data
    addAttendance(name, nic, age, phone, id);
  };
  
  

  return (
    <div>
      <h1 class="heading">Biometric Attendance System</h1>

      <hr></hr>
      
      <button className="btn btn-primary" onClick={handleAddAttendance}>Mark Attendance</button>
      
      <hr></hr>

      <div>
        
      </div>

      
      <table className="table">
        <thead className="thead-dark">
          <tr>
            <th scope="col">Order</th>
            <th scope="col">Id</th>
            <th scope="col">Name</th>
            <th scope="col">NIC Number</th>
            <th scope="col">Age</th>
            <th scope="col">Phone Number</th>
            <th scope="col">Attendance Time</th>
            <th scope="col">Attendance Date</th>
          </tr>
        </thead>
        <tbody>
          {attendanceData.map((entry, index) => (
            <tr key={index}>
              <th scope="row">{index + 1}</th>
              <td>{entry.id}</td>
              <td>{entry.name}</td>
              <td>{entry.nic}</td>
              <td>{entry.age}</td>
              <td>{entry.phoneNumber}</td>
              <td>{entry.attendanceTime}</td>
              <td>{entry.attendanceDate}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
};

export default AttendanceSystem;
