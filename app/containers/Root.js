// @flow
import React from 'react';
import { BrowserRouter as Router } from 'react-router-dom';
import { hot } from 'react-hot-loader/root';
import Routes from '../Routes';

const Root = () => (
  <Router>
    <Routes />
  </Router>
);

export default hot(Root);
