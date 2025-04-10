import React from 'react';

const AboutPage = () => {
  return (
    <div className="container mt-5">
      <h2>About the Biometric Access Control System</h2>
      <p>
        This project integrates a fingerprint scanner, solenoid lock, ESP32, and Firebase.
        It authenticates users, tracks access attempts, and logs data in real time.
      </p>
      <p>
        Admins can monitor access logs and manage settings via a web dashboard.
      </p>
      <p>
        Developed by the DoorLock Project Team — combining hardware and cloud technology.
      </p>
    </div>
  );
};

export default AboutPage;
