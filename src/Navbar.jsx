import React from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { signOut } from 'firebase/auth';
import { auth } from './firebase';

const Navbar = () => {
  const navigate = useNavigate();

  const handleLogout = async () => {
    await signOut(auth);
    navigate('/');
  };

  return (
    <nav className="navbar navbar-expand-lg navbar-dark bg-dark px-3">
      <Link className="navbar-brand" to="/logs">DoorLock System</Link>
      <div className="navbar-nav ms-auto">
        <Link className="nav-link" to="/logs">Logs</Link>
        <Link className="nav-link" to="/admin">Admin Controller</Link>
        <button className="btn btn-outline-light ms-3" onClick={handleLogout}>Sign Out</button>
      </div>
    </nav>
  );
};

export default Navbar;
