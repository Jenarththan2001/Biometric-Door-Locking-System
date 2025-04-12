import React from 'react';
import { Link } from 'react-router-dom';  // Import Link from react-router-dom
import '../style.css';
import twitterIcon from '../assets/img/twitter.png';
import fbIcon from '../assets/img/facebook.png';
import hIcon from '../assets/img/heart.png';
import gpIcon from '../assets/img/google-plus.png';
import searchIcon from '../assets/img/search.png';
import shopIcon from '../assets/img/shopping-cart.png';
import bgIcon from '../assets/img/door.jpg';
import esp32Icon from '../assets/img/esp32.jpg';
import cdIcon from '../assets/img/cd.png';
import fpIcon from '../assets/img/fingerprintsensor.jpg';
import oldIcon from '../assets/img/old.jpg';
import solonoidIcon from '../assets/img/solonoid.jpg';
import lm from '../assets/img/LM2596.jpg';
import relay from '../assets/img/relay.jpg';
import rtc from '../assets/img/RTC.jpg';
import led from '../assets/img/led.jpg';




function MainPage() {
  return (
    <div>
      {/* navbar top */}
      <div className="container">
        <div className="navbar-top">
          <div className="social-link">
            <i><img src={twitterIcon} alt="" width="30px" /></i>
            <i><img src={fbIcon} alt="" width="30px" /></i>
            <i><img src={gpIcon} alt="" width="30px" /></i>
          </div>
          <div className="logo">
            <h3>Biometric Door Locking and Attendance System</h3>
          </div>
          <div className="icons">
            <i><img src={searchIcon} alt="" width="20px" /></i>
            <i><img src={hIcon} alt="" width="20px" /></i>
            <i><img src={shopIcon} alt="" width="25px" /></i>
          </div>
        </div>
      </div>
      {/* navbar top */}

      {/* main content */}
      <div className="main-content">
        <nav className="navbar navbar-expand-md" id="navbar-color">
          <div className="container">
            {/* Toggler/collapsibe Button */}
            <button className="navbar-toggler" type="button" data-toggle="collapse" data-target="#collapsibleNavbar">
              <i><img src={bgIcon} alt="" width="30px" /></i>
            </button>

            {/* Navbar links */}
            <div className="collapse navbar-collapse" id="collapsibleNavbar">
              <ul className="navbar-nav">
                <li className="nav-item"><a className="nav-link" href="#">Home</a></li>
                <li className="nav-item"><a className="nav-link" href="#">Attendance Table</a></li>
                <li className="nav-item"><a className="nav-link" href="#">Registration</a></li>
                <li className="nav-item"><a className="nav-link" href="#">Components</a></li>
                <li className="nav-item"><a className="nav-link" href="#">Team</a></li>
                <li className="nav-item"><a className="nav-link" href="#">Contact</a></li>
              </ul>
            </div>
          </div>
        </nav>

        <div className="content">
          <h1 className="heading">Bio Guard Attendance <br /> </h1>
          <p className="biometric-text">
            The Biometric Fingerprint and Attendance System uses fingerprint authentication to verify identity and record attendance.
            Authorized users gain access upon successful verification, while attendance is logged in real-time. This ensures secure, accurate,
            and tamper-proof attendance management for workplaces and institutions.
          </p>
          {/* Link for navigation */}
          <div id="btn1">
            <Link to="/attendancesystem">
              <button>Attendance</button>
            </Link>
          </div>
        </div>
      </div>
      {/* main content */}

      <div class = "heading">
        <h1>Electrical Components</h1>
      </div>

      {/* Card Section */}
      <div className="row" style={{ marginTop: '50px' }}>
        <div className="col-md-3 py-3 py-md-0">
          <div className="card" id="c">
            <img src={esp32Icon} alt="" className="card image-top" />
            <div className="card-body">
              <h3 className="card-titel text-center">ESP32-DEV</h3>
              <p className="card-text text-center"></p>
              <div id="btn3"><button>Specification</button></div>
            </div>
          </div>
        </div>
        {/* Add other cards as well */}
        <div className="col-md-3 py-3 py-md-0">
          <div className="card" id="c">
            <img src={fpIcon} alt="" className="card image-top" />
            <div className="card-body">
              <h3 className="card-titel text-center">R307 Fingerprin Sensor</h3>
              <p className="card-text text-center"></p>
              <div id="btn3"><button>Specification</button></div>
            </div>
          </div>
        </div>
        {/* Add other cards as well */}
        <div className="col-md-3 py-3 py-md-0">
          <div className="card" id="c">
            <img src={oldIcon} alt="" className="card image-top" />
            <div className="card-body">
              <h3 className="card-titel text-center">OLED Display</h3>
              <p className="card-text text-center"></p>
              <div id="btn3"><button>Specification</button></div>
            </div>
          </div>
        </div>
        {/* Add other cards as well */}
        <div className="col-md-3 py-3 py-md-0">
          <div className="card" id="c">
            <img src={solonoidIcon} alt="" className="card image-top" />
            <div className="card-body">
              <h3 className="card-titel text-center">Solenoid Lock</h3>
              <p className="card-text text-center"></p>
              <div id="btn3"><button>Specification</button></div>
            </div>
          </div>
        </div>

      </div>
      {/* card3 */}

      {/* Card Section */}
      <div className="row" style={{ marginTop: '50px' }}>
        <div className="col-md-3 py-3 py-md-0">
          <div className="card" id="c">
            <img src={led} alt="" className="card image-top" />
            <div className="card-body">
              <h3 className="card-titel text-center">LEDs</h3>
              <p className="card-text text-center"></p>
              <div id="btn3"><button>Specification</button></div>
            </div>
          </div>
        </div>
        {/* Add other cards as well */}
        <div className="col-md-3 py-3 py-md-0">
          <div className="card" id="c">
            <img src={rtc} alt="" className="card image-top" />
            <div className="card-body">
              <h3 className="card-titel text-center">DS1307 RTC</h3>
              <p className="card-text text-center"></p>
              <div id="btn3"><button>Specification</button></div>
            </div>
          </div>
        </div>
        {/* Add other cards as well */}
        <div className="col-md-3 py-3 py-md-0">
          <div className="card" id="c">
            <img src={relay} alt="" className="card image-top" />
            <div className="card-body">
              <h3 className="card-titel text-center">Relay module</h3>
              <p className="card-text text-center"></p>
              <div id="btn3"><button>Specification</button></div>
            </div>
          </div>
        </div>
        {/* Add other cards as well */}
        <div className="col-md-3 py-3 py-md-0">
          <div className="card" id="c">
            <img src={lm} alt="" className="card image-top" />
            <div className="card-body">
              <h3 className="card-titel text-center">LM 2596</h3>
              <p className="card-text text-center"></p>
              <div id="btn3"><button>Specification</button></div>
            </div>
          </div>
        </div>

      </div>

      

      {/* About Section */}
      <div className="container">
        <h1 className="text-center" style={{ marginTop: '50px' }}>ABOUT</h1>
        <div className="row" style={{ marginTop: '50px' }}>
          <div className="col-md-6 py-3 py-md-0">
            <div className="card">
              <img src={cdIcon} alt="" />
            </div>
          </div>
          <div className="col-md-6 py-3 py-md-0">
            <p>The Biometric Door Locking and Attendance System is an advanced security and attendance solution that integrates a fingerprint sensor,
              solenoid lock, and web application using an ESP32 microcontroller. The system ensures secure access control with R307 fingerprint
              authentication and real-time attendance tracking. It manages access through a solenoid lock and syncs data to a web platform for easy
              monitoring. Key components include the ESP32 board, fingerprint sensor, solenoid lock, and an OLED display for system status. This solution
              is ideal for organizations seeking efficient attendance management and high-level security.</p>
            <div id="btn4"><button>Read More...</button></div>
          </div>
        </div>
      </div>
      {/* about */}

      {/* footer */}
      <footer id="footer">
        <h1 className="text-center">Bio-GUARD ATTENDANCE</h1>
        <p className="text-center">Safe System from unknown Dangers</p>
        <div className="icons text-center">
          <i className="bx bxl-twitter"></i>
          <i className="bx bxl-facebook"></i>
          <i className="bx bxl-google"></i>
          <i className="bx bxl-skype"></i>
          <i className="bx bxl-instagram"></i>
        </div>
        <div className="copyright text-center">
          &copy; Copyright <strong><span>Team-jfn</span></strong>. All Rights Reserved
        </div>
        <div className="credite text-center">
          Designed By <a href="#">NATAN@ALWAYS</a>
        </div>
      </footer>
      {/* footer */}
    </div>
  );
}

export default MainPage;
