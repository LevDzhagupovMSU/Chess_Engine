const boardElement = document.getElementById('board');
const statusElement = document.getElementById('status');
const turnElement = document.getElementById('turn');
const turnBadgeElement = document.getElementById('turn-badge');
const lastActionElement = document.getElementById('last-action');
const connectBtn = document.getElementById('connect-btn');
const newGameBtn = document.getElementById('new-game-btn');
const wsUrlInput = document.getElementById('ws-url');

const pieceSymbols = {
  'white-pawn': '♙',
  'white-rook': '♖',
  'white-knight': '♘',
  'white-bishop': '♗',
  'white-queen': '♕',
  'white-king': '♔',
  'black-pawn': '♟',
  'black-rook': '♜',
  'black-knight': '♞',
  'black-bishop': '♝',
  'black-queen': '♛',
  'black-king': '♚',
};

let state = {
  board: [],
  selectedSquare: null,
  turn: 'white',
  connected: false,
  ws: null,
  awaitingResponse: false,
  lastMove: null,
};

function createInitialBoard() {
  return [
    'black-rook', 'black-knight', 'black-bishop', 'black-queen', 'black-king', 'black-bishop', 'black-knight', 'black-rook',
    'black-pawn', 'black-pawn', 'black-pawn', 'black-pawn', 'black-pawn', 'black-pawn', 'black-pawn', 'black-pawn',
    null, null, null, null, null, null, null, null,
    null, null, null, null, null, null, null, null,
    null, null, null, null, null, null, null, null,
    null, null, null, null, null, null, null, null,
    'white-pawn', 'white-pawn', 'white-pawn', 'white-pawn', 'white-pawn', 'white-pawn', 'white-pawn', 'white-pawn',
    'white-rook', 'white-knight', 'white-bishop', 'white-queen', 'white-king', 'white-bishop', 'white-knight', 'white-rook',
  ];
}

function init() {
  state.board = createInitialBoard();
  renderBoard();
  bindEvents();
}

function bindEvents() {
  connectBtn.addEventListener('click', connectToServer);
  newGameBtn.addEventListener('click', resetGame);
}

function renderBoard() {
  boardElement.innerHTML = '';

  state.board.forEach((piece, index) => {
    const square = document.createElement('button');
    square.className = `square ${isLightSquare(index) ? 'light' : 'dark'}`;
    square.dataset.index = index;

    if (state.selectedSquare === index) {
      square.classList.add('selected');
    }

    if (piece) {
      const pieceEl = document.createElement('span');
      pieceEl.className = 'piece';
      pieceEl.dataset.color = piece.startsWith('black') ? 'black' : 'white';
      pieceEl.textContent = pieceSymbols[piece] || '?';
      square.appendChild(pieceEl);
    }

    square.addEventListener('click', () => onSquareClick(index));
    boardElement.appendChild(square);
  });
}

function isLightSquare(index) {
  const row = Math.floor(index / 8);
  const col = index % 8;
  return (row + col) % 2 === 0;
}

function resetGame() {
  state.board = createInitialBoard();
  state.selectedSquare = null;
  state.turn = 'white';
  updateTurnLabels();
  updateStatus('Новая партия началась');
  renderBoard();
}

function onSquareClick(index) {
  if (state.awaitingResponse) {
    updateStatus('Ждём ответ сервера, подождите');
    return;
  }

  if (state.selectedSquare === null) {
    const piece = state.board[index];
    if (piece && piece.startsWith(state.turn)) {
      state.selectedSquare = index;
      renderBoard();
      return;
    }
    updateStatus('Выберите свою фигуру');
    return;
  }

  const from = state.selectedSquare;
  const to = index;

  if (from === to) {
    state.selectedSquare = null;
    renderBoard();
    return;
  }

  applyMove(from, to);
  state.selectedSquare = null;
  renderBoard();
}

function applyMove(from, to) {
  const movingPiece = state.board[from];
  const targetPiece = state.board[to];

  if (!movingPiece) {
    return;
  }

  if (targetPiece && targetPiece.startsWith(state.turn)) {
    updateStatus('Нельзя съесть свою фигуру');
    return;
  }

  const movePayload = {
    type: 'player',
    move: [from, to],
  };

  if (!state.connected || !state.ws || state.ws.readyState !== WebSocket.OPEN) {
    updateStatus('Сервер недоступен — ход не отправлен');
    return;
  }

  state.awaitingResponse = true;
  
  // СОХРАНЯЕМ ДАННЫЕ ДЛЯ ОТКАТА
  state.lastMove = { 
    from, 
    to, 
    piece: movingPiece, 
    captured: targetPiece 
  };
  
  // ПЕРЕМЕЩАЕМ ФИГУРУ НА ДОСКЕ
  state.board[to] = movingPiece;
  state.board[from] = null;
  
  // МЕНЯЕМ ОЧЕРЕДЬ
  state.turn = 'black';
  updateTurnLabels();
  state.selectedSquare = null;
  renderBoard();
  
  updateStatus(`Отправлен ход: ${indexToCoordinate(from)} → ${indexToCoordinate(to)}`);
  lastActionElement.textContent = `Ход отправлен: ${indexToCoordinate(from)} → ${indexToCoordinate(to)}`;

  state.ws.send(JSON.stringify(movePayload));
}

function indexToCoordinate(index) {
  const row = 8 - Math.floor(index / 8);
  const col = String.fromCharCode(97 + (index % 8));
  return `${col}${row}`;
}

function connectToServer() {
  if (state.ws && state.ws.readyState === WebSocket.OPEN) {
    state.ws.close();
  }

  const url = wsUrlInput.value.trim() || 'ws://127.0.0.1:8080';
  updateStatus('Подключение...');

  try {
    state.ws = new WebSocket(url);
  } catch (error) {
    updateStatus(`Ошибка подключения: ${error.message}`);
    return;
  }

  state.ws.addEventListener('open', () => {
    state.connected = true;
    statusElement.textContent = 'Подключено';
    statusElement.className = 'value online';
    updateStatus('Подключено к серверу');
  });

  state.ws.addEventListener('message', (event) => {
    try {
      const payload = JSON.parse(event.data);
      handleServerMessage(payload);
    } catch (error) {
      updateStatus('Сервер прислал невалидное сообщение');
    }
  });

  state.ws.addEventListener('close', () => {
    state.connected = false;
    statusElement.textContent = 'Оффлайн';
    statusElement.className = 'value offline';
    updateStatus('Соединение закрыто');
  });

  state.ws.addEventListener('error', () => {
    state.connected = false;
    statusElement.textContent = 'Ошибка';
    statusElement.className = 'value offline';
    updateStatus('Не удалось подключиться к WebSocket');
  });
}

function handleServerMessage(payload) {
  if (payload.type === 'ai' && Array.isArray(payload.move)) {
    const [from, to] = payload.move;
    const movingPiece = state.board[from];

    if (movingPiece) {
      state.board[to] = movingPiece;
      state.board[from] = null;
    }

    state.awaitingResponse = false;
    state.turn = 'white';
    updateTurnLabels();
    updateStatus(`Ход ИИ: ${indexToCoordinate(from)} → ${indexToCoordinate(to)}`);
    lastActionElement.textContent = `Ход ИИ: ${indexToCoordinate(from)} → ${indexToCoordinate(to)}`;
    renderBoard();
    return;
  }

  if (payload.type === 'bad move') {
    if (state.lastMove) {
      state.board[state.lastMove.from] = state.lastMove.piece;
      state.board[state.lastMove.to] = state.lastMove.captured || null;
    }
    state.awaitingResponse = false;
    state.turn = 'white';
    updateTurnLabels();
    renderBoard();
    updateStatus('Неверный ход, попробуйте ещё раз');
    lastActionElement.textContent = 'Неверный ход';
    return;
  }

  if (payload.type === 'result') {
    state.awaitingResponse = false;
    updateStatus(`Игра окончена: ${payload.result}`);
    lastActionElement.textContent = `Игра окончена: ${payload.result}`;
    return;
  }

  if (payload.type === 'state') {
    updateStatus(`Сервер прислал состояние: ${payload.status || 'ok'}`);
    return;
  }

  updateStatus('Сервер прислал неизвестное сообщение');
}

function updateTurnLabels() {
  turnElement.textContent = state.turn === 'white' ? 'Белые' : 'Черные';
  if (turnBadgeElement) {
    turnBadgeElement.textContent = state.turn === 'white' ? 'Белые' : 'Черные';
  }
}

function updateStatus(message) {
  lastActionElement.textContent = message;
}

init();
