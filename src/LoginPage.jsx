import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { auth } from './firebase';
import { signInWithEmailAndPassword } from 'firebase/auth';

const LoginPage = () => {
  const navigate = useNavigate();
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');

  const handleLogin = async (e) => {
    e.preventDefault();
    try {
      await signInWithEmailAndPassword(auth, email, password);
      navigate('/logs'); // Redirect to main page
    } catch (err) {
      setError('Invalid credentials');
    }
  };

  return (
    <div className="d-flex" 
    style={{
        height: '100vh',
        backgroundImage: 'linear-gradient(rgba(0,0,0,0.5), rgba(0,0,0,0.5)), url("/images/bg-login.jpg")',
        backgroundSize: 'cover',
        backgroundPosition: 'center',
        backgroundRepeat: 'no-repeat',
        color: 'white',
      }}
      
    >

    {/* Page Heading */}
    <div
    className="text-center py-4"
    style={{
        backgroundImage: 'url("/images/login.png")',
        backgroundSize: 'cover',
        backgroundPosition: 'center',
        backgroundRepeat: 'no-repeat',
    }}
    >
    <h1
        className="text-shadow"
        style={{
        fontWeight: 'bold',
        fontSize: '2.5rem',
        background: 'linear-gradient(to right, #00c6ff, #0072ff)',
        WebkitBackgroundClip: 'text',
        WebkitTextFillColor: 'transparent',
        }}
    >
        BioFinger Door Locking System
    </h1>
    </div>


      {/* Login Section */}

      <div className="w-50 d-flex align-items-center justify-content-center bg-light"
        style={{
            minHeight: '100vh',
            backgroundImage: 'url("/images/log2.png")',
            backgroundSize: 'cover',
            backgroundPosition: 'center',
        }}
      >
        <form onSubmit={handleLogin} className="bg-white p-4 rounded shadow" style={{ width: '80%', maxWidth: '300px' }}>
          <h3 className="text-center text-primary mb-4">Admin Login</h3>
          <input
            type="email"
            placeholder="Email"
            className="form-control my-2"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
            required
          />
          <input
            type="password"
            placeholder="Password"
            className="form-control my-2"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            required
          />
          <button className="btn btn-primary w-100">Login</button>
          {error && <p className="text-danger mt-3 text-center">{error}</p>}
        </form>
      </div>

      {/* About Project Section */}
      <div className="w-50 d-flex align-items-center justify-content-center bg-secondary text-white" 
        style={{
            minHeight: '100vh',
            backgroundImage: 'url("/images/circuit.png")',
            backgroundSize: 'cover',
            backgroundPosition: 'center',
            
            zIndex: 0,
            
            
            transform: 'scale(1.1)',
          }}
      >
        <div className="text-center">
        <div
            style={{
            backgroundColor: '#f0f0f0', // light gray or choose your color
            padding: '16px 24px',
            borderRadius: '8px',
            display: 'inline-block',
            fontWeight: 'bold',
            boxShadow: '0 4px 12px rgba(0,0,0,0.1)',
            }}
        >
            <h2 className="text-dark">Know More About the Project</h2>
        </div>
            <button
                className="btn btn-dark px-4 py-2 mt-3"
                onClick={() => navigate('/about')}
            >
                View Project Description
            </button>
        </div>
      </div>
    </div>
  );
};

export default LoginPage;
