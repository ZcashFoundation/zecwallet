/* eslint-disable react/prop-types */
import React, { Component } from 'react';
import { withRouter } from 'react-router-dom';
import {
  AccordionItemButton,
  AccordionItem,
  AccordionItemHeading,
  AccordionItemPanel,
  Accordion
} from 'react-accessible-accordion';
import styles from './Addressbook.module.css';
import cstyles from './Common.module.css';
import { AddressBookEntry } from './AppState';
import ScrollPane from './ScrollPane';
import Utils from '../utils/utils';
import { ZcashURITarget } from '../utils/uris';
import routes from '../constants/routes.json';

// Internal because we're using withRouter just below
const AddressBookItemInteral = ({ item, removeAddressBookEntry, setSendTo, history }) => {
  return (
    <AccordionItem key={item.label} className={[cstyles.well, cstyles.margintopsmall].join(' ')} uuid={item.label}>
      <AccordionItemHeading>
        <AccordionItemButton className={cstyles.accordionHeader}>
          <div className={[cstyles.flexspacebetween].join(' ')}>
            <div>{item.label}</div>
            <div>{item.address}</div>
          </div>
        </AccordionItemButton>
      </AccordionItemHeading>
      <AccordionItemPanel>
        <div className={[cstyles.well, styles.addressbookentrybuttons].join(' ')}>
          <button
            type="button"
            className={cstyles.primarybutton}
            onClick={() => {
              setSendTo(new ZcashURITarget(item.address, null, null));
              history.push(routes.SEND);
            }}
          >
            Send To
          </button>
          <button type="button" className={cstyles.primarybutton} onClick={() => removeAddressBookEntry(item.label)}>
            Delete
          </button>
        </div>
      </AccordionItemPanel>
    </AccordionItem>
  );
};
const AddressBookItem = withRouter(AddressBookItemInteral);

type Props = {
  addressBook: AddressBookEntry[],
  addAddressBookEntry: (label: string, address: string) => void,
  removeAddressBookEntry: (label: string) => void,
  setSendTo: (targets: ZcashURITarget[] | ZcashURITarget) => void
};

type State = {
  currentLabel: string,
  currentAddress: string,
  addButtonEnabled: boolean
};

export default class AddressBook extends Component<Props, State> {
  constructor(props) {
    super(props);

    this.state = { currentLabel: '', currentAddress: '', addButtonEnabled: false };
  }

  updateLabel = (currentLabel: string) => {
    // Don't update the field if it is longer than 20 chars
    if (currentLabel.length > 20) return;

    const { currentAddress } = this.state;
    this.setState({ currentLabel });

    const { labelError, addressIsValid } = this.validate(currentLabel, currentAddress);
    this.setAddButtonEnabled(!labelError && addressIsValid && currentLabel !== '' && currentAddress !== '');
  };

  updateAddress = (currentAddress: string) => {
    const { currentLabel } = this.state;
    this.setState({ currentAddress });

    const { labelError, addressIsValid } = this.validate(currentLabel, currentAddress);

    this.setAddButtonEnabled(!labelError && addressIsValid && currentLabel !== '' && currentAddress !== '');
  };

  addButtonClicked = () => {
    const { addAddressBookEntry } = this.props;
    const { currentLabel, currentAddress } = this.state;

    addAddressBookEntry(currentLabel, currentAddress);
    this.setState({ currentLabel: '', currentAddress: '' });
  };

  setAddButtonEnabled = (addButtonEnabled: boolean) => {
    this.setState({ addButtonEnabled });
  };

  validate = (currentLabel, currentAddress) => {
    const { addressBook } = this.props;

    let labelError = addressBook.find(i => i.label === currentLabel) ? 'Duplicate Label' : null;
    labelError = currentLabel.length > 12 ? 'Label is too long' : labelError;

    const addressIsValid =
      currentAddress === '' || Utils.isZaddr(currentAddress) || Utils.isTransparent(currentAddress);

    return { labelError, addressIsValid };
  };

  render() {
    const { addressBook, removeAddressBookEntry, setSendTo } = this.props;
    const { currentLabel, currentAddress, addButtonEnabled } = this.state;

    const { labelError, addressIsValid } = this.validate(currentLabel, currentAddress);

    return (
      <div>
        <div className={[cstyles.xlarge, cstyles.padall, cstyles.center].join(' ')}>Address Book</div>

        <div className={styles.addressbookcontainer}>
          <div className={[cstyles.well].join(' ')}>
            <div className={[cstyles.flexspacebetween].join(' ')}>
              <div className={cstyles.sublight}>Label</div>
              <div className={cstyles.validationerror}>
                {!labelError ? (
                  <i className={[cstyles.green, 'fas', 'fa-check'].join(' ')} />
                ) : (
                  <span className={cstyles.red}>{labelError}</span>
                )}
              </div>
            </div>
            <input
              type="text"
              value={currentLabel}
              className={[cstyles.inputbox, cstyles.margintopsmall].join(' ')}
              onChange={e => this.updateLabel(e.target.value)}
            />

            <div className={cstyles.margintoplarge} />

            <div className={[cstyles.flexspacebetween].join(' ')}>
              <div className={cstyles.sublight}>Address</div>
              <div className={cstyles.validationerror}>
                {addressIsValid ? (
                  <i className={[cstyles.green, 'fas', 'fa-check'].join(' ')} />
                ) : (
                  <span className={cstyles.red}>Invalid Address</span>
                )}
              </div>
            </div>
            <input
              type="text"
              value={currentAddress}
              className={[cstyles.inputbox, cstyles.margintopsmall].join(' ')}
              onChange={e => this.updateAddress(e.target.value)}
            />

            <div className={cstyles.margintoplarge} />

            <button
              type="button"
              className={cstyles.primarybutton}
              disabled={!addButtonEnabled}
              onClick={this.addButtonClicked}
            >
              Add
            </button>
          </div>

          <ScrollPane offsetHeight={300}>
            <div className={styles.addressbooklist}>
              <div className={[cstyles.flexspacebetween, cstyles.tableheader, cstyles.sublight].join(' ')}>
                <div>Label</div>
                <div>Address</div>
              </div>
              {addressBook && (
                <Accordion>
                  {addressBook.map(item => (
                    <AddressBookItem
                      key={item.label}
                      item={item}
                      removeAddressBookEntry={removeAddressBookEntry}
                      setSendTo={setSendTo}
                    />
                  ))}
                </Accordion>
              )}
            </div>
          </ScrollPane>
        </div>
      </div>
    );
  }
}
