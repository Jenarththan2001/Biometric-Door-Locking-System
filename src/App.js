import React from 'react';
import { Routes, Route } from 'react-router-dom';
import LoginPage from './LoginPage';
import AboutPage from './AboutPage';
import LogTable from './LogTable';
import AdminController from './AdminController';

function App() {
  return (
    <Routes>
      <Route path="/" element={<LoginPage />} />
      <Route path="/about" element={<AboutPage />} />
      <Route path="/logs" element={<LogTable />} />
      <Route path="/admin" element={<AdminController />} />
    </Routes>
  );
}

export default App;
