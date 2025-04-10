import React from 'react';
import Navbar from './Navbar';

const AdminController = () => {
  return (
    <div>
      <Navbar />
      <div className="container mt-5">
        <h2>Admin Control Panel</h2>
        <button className="btn btn-success m-2">Add Person</button>
        <button className="btn btn-danger m-2">Delete Person</button>
        <button className="btn btn-warning m-2">Edit Person</button>
      </div>
    </div>
  );
};

export default AdminController;
