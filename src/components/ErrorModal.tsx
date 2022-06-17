import { ReactElement } from "react";
import Modal from "react-modal";
import cstyles from "./Common.module.css";

export class ErrorModalData {
  title: string;
  body: string | ReactElement;
  modalIsOpen: boolean;
  closeModal?: () => void;

  constructor() {
    this.title = "";
    this.body = "";
    this.modalIsOpen = false;
  }
}

export const ErrorModal = ({
  title,
  body,
  modalIsOpen,
  closeModal,
}: ErrorModalData) => {
  return (
    <Modal
      isOpen={modalIsOpen}
      onRequestClose={closeModal}
      className={cstyles.modal}
      overlayClassName={cstyles.modalOverlay}
    >
      <div className={[cstyles.verticalflex].join(" ")}>
        <div
          className={cstyles.marginbottomlarge}
          style={{ textAlign: "center" }}
        >
          {title}
        </div>

        <div
          className={cstyles.well}
          style={{
            textAlign: "center",
            wordBreak: "break-all",
            maxHeight: "400px",
            overflowY: "auto",
          }}
        >
          {body}
        </div>
      </div>

      <div className={cstyles.buttoncontainer}>
        <button
          type="button"
          className={cstyles.primarybutton}
          onClick={closeModal}
        >
          Close
        </button>
      </div>
    </Modal>
  );
};
