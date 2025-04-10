import React, { useEffect, useState } from 'react';
import { ref, onChildAdded } from 'firebase/database';
import { db, auth } from './firebase';
import Navbar from './Navbar';

const LogTable = () => {
  const [logs, setLogs] = useState([]);

  // // Helper: Convert UNIX timestamp to readable format
  // const formatTimestamp = (ts) => {
  //   // If it's in seconds, convert to milliseconds
  //   if (ts < 1000000000000) {
  //     ts = ts * 1000;
  //   }
  //   const date = new Date(ts);
  //   return date.toLocaleString('en-US', {
  //     dateStyle: 'medium',
  //     timeStyle: 'short'
  //   });
  // };

  useEffect(() => {
    const logsRef = ref(db, 'user_input');
    onChildAdded(logsRef, (snapshot) => {
      const rawLog = snapshot.val();
      const formattedLog = {
        userId: `EMP${rawLog.UserID}`,
        status: rawLog.AccessGranted === 1 ? 'granted' : 'rejected',
        timestamp: `${rawLog.Timestamp}`
      };

      setLogs((prevLogs) => {
        // Check if this userId + timestamp combo already exists
        const exists = prevLogs.some(
          log => log.userId === formattedLog.userId && log.timestamp === formattedLog.timestamp
        );

        if (exists) return prevLogs; // Don't add duplicate

        const updatedLogs = [...prevLogs, formattedLog];
        updatedLogs.sort((a, b) => b.timestamp.localeCompare(a.timestamp));
        return updatedLogs;
      });
    });
  }, []);

  return (
    <>
      <Navbar />
      <div className="container mt-4">
        <h2 className="mb-3">Fingerprint Access Logs</h2>

        <div className="mb-3">
          <span className="badge bg-success me-2">
            Granted: {logs.filter(log => log.status === 'granted').length}
          </span>
          <span className="badge bg-danger">
            Rejected: {logs.filter(log => log.status === 'rejected').length}
          </span>
        </div>

        <table className="table table-bordered table-striped">
          <thead className="table-dark">
            <tr>
              <th>User ID</th>
              <th>Status</th>
              <th>Timestamp</th>
            </tr>
          </thead>
          <tbody>
            {logs.map((log, index) => (
              <tr key={index}>
                <td>{log.userId}</td>
                <td className={log.status === 'granted' ? 'text-success' : 'text-danger'}>
                  {log.status}
                </td>
                <td>{log.timestamp}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </>
  );
};

export default LogTable;
