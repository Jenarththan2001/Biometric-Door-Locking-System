import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';  // Import Router and Routes
import './index.css';
import App from './App.jsx';
import AttendanceSystem from './components/AttendanceSystem.jsx';
import MainPage from './components/MainPage.jsx';

createRoot(document.getElementById('root')).render(
  <StrictMode>
    <Router>
      {/* Wrap everything with Router */}
      <Routes>
        {/* Define routes for each component */}
        <Route path="/" element={<MainPage />} />
        <Route path="/attendancesystem" element={<AttendanceSystem />} />
      </Routes>
    </Router>
  </StrictMode>
);
