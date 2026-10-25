const boardElement = document.getElementById('board');
const statusElement = document.getElementById('status');
const turnElement = document.getElementById('turn');
const turnBadgeElement = document.getElementById('turn-badge');
const lastActionElement = document.getElementById('last-action');
const connectBtn = document.getElementById('connect-btn');
const newGameBtn = document.getElementById('new-game-btn');
const wsUrlInput = document.getElementById('ws-url');
const historyListElement = document.getElementById('move-history');
const historyEmptyElement = document.getElementById('history-empty');

const symbols = {
  'white-pawn': '♙', 'white-rook': '♖', 'white-knight': '♘',
  'white-bishop': '♗', 'white-queen': '♕', 'white-king': '♔',
  'black-pawn': '♟', 'black-rook': '♜', 'black-knight': '♞',
  'black-bishop': '♝', 'black-queen': '♛', 'black-king': '♚',
};

const initialPosition = [
  'black-rook', 'black-knight', 'black-bishop', 'black-queen', 'black-king', 'black-bishop', 'black-knight', 'black-rook',
  ...Array(8).fill('black-pawn'), ...Array(32).fill(null),
  ...Array(8).fill('white-pawn'),
  'white-rook', 'white-knight', 'white-bishop', 'white-queen', 'white-king', 'white-bishop', 'white-knight', 'white-rook',
];

const state = {
  board: [...initialPosition],
  selected: null,
  turn: 'white',
  ws: null,
  awaiting: false,
  gameOver: false,
  pendingMove: null,
  lastMove: null,
  history: [],
};

function coordinate(index) {
  return `${String.fromCharCode(97 + index % 8)}${8 - Math.floor(index / 8)}`;
}

function setStatus(message, connection = null) {
  lastActionElement.textContent = message;
  if (connection) {
    statusElement.textContent = connection.label;
    statusElement.className = `value ${connection.className}`;
  }
}

function updateTurn() {
  const whiteToMove = state.turn === 'white';
  const label = whiteToMove ? 'Белые' : 'Чёрные';
  turnElement.textContent = label;
  turnBadgeElement.textContent = label;
  document.getElementById('white-player').classList.toggle('active-player', whiteToMove);
  document.getElementById('black-player').classList.toggle('active-player', !whiteToMove);
}

function renderBoard() {
  boardElement.replaceChildren();
  state.board.forEach((piece, index) => {
    const square = document.createElement('button');
    const row = Math.floor(index / 8);
    const col = index % 8;
    square.type = 'button';
    square.className = `square ${(row + col) % 2 === 0 ? 'light' : 'dark'}`;
    square.dataset.index = index;
    square.setAttribute('aria-label', `${coordinate(index)}${piece ? `, ${piece.replace('-', ' ')}` : ', пустая клетка'}`);
    square.disabled = state.awaiting || state.gameOver || !state.ws || state.ws.readyState !== WebSocket.OPEN;
    if (state.selected === index) square.classList.add('selected');
    if (state.lastMove && (state.lastMove.from === index || state.lastMove.to === index)) square.classList.add('last-move');
    if (piece) {
      const pieceElement = document.createElement('span');
      pieceElement.className = 'piece';
      pieceElement.dataset.color = piece.startsWith('black') ? 'black' : 'white';
      pieceElement.textContent = symbols[piece];
      square.append(pieceElement);
    }
    square.addEventListener('click', () => onSquareClick(index));
    boardElement.append(square);
  });
}

function resetPosition() {
  state.board = [...initialPosition];
  state.selected = null;
  state.turn = 'white';
  state.awaiting = false;
  state.gameOver = false;
  state.pendingMove = null;
  state.lastMove = null;
  state.history = [];
  renderHistory();
  updateTurn();
  renderBoard();
}

function formatHistoryMove(move, color) {
  const piece = state.board[move.from];
  if (!piece) return `${coordinate(move.from)}–${coordinate(move.to)}`;
  const kind = piece.slice(color.length + 1);
  const target = state.board[move.to];
  const enPassant = kind === 'pawn' && move.from % 8 !== move.to % 8 && !target;
  const isCapture = Boolean(target) || enPassant;
  if (kind === 'king' && Math.abs((move.from % 8) - (move.to % 8)) === 2) {
    return move.to % 8 > move.from % 8 ? 'O–O' : 'O–O–O';
  }
  const piecePrefix = { pawn: '', knight: 'К', bishop: 'С', rook: 'Л', queen: 'Ф', king: 'Кр' }[kind] || '';
  const promotion = kind === 'pawn' && (Math.floor(move.to / 8) === 0 || Math.floor(move.to / 8) === 7)
    ? `=${({ queen: 'Ф', rook: 'Л', bishop: 'С', knight: 'К' }[move.promotion || 'queen'])}` : '';
  const pawnFile = kind === 'pawn' && isCapture ? String.fromCharCode(97 + move.from % 8) : '';
  return `${piecePrefix}${pawnFile}${isCapture ? '×' : '–'}${coordinate(move.to)}${promotion}`;
}

function recordHistoryMove(move, color) {
  if (color === 'white') state.history.push({ number: state.history.length + 1, white: formatHistoryMove(move, color), black: '' });
  else if (state.history.length) state.history[state.history.length - 1].black = formatHistoryMove(move, color);
  renderHistory();
}

function renderHistory() {
  historyListElement.replaceChildren();
  historyEmptyElement.hidden = state.history.length > 0;
  state.history.forEach((entry) => {
    const row = document.createElement('li');
    row.className = 'history-row';
    const number = document.createElement('span');
    number.className = 'history-number';
    number.textContent = entry.number;
    const white = document.createElement('span');
    white.className = 'history-move';
    white.textContent = entry.white;
    const black = document.createElement('span');
    black.className = `history-move${entry.black ? '' : ' pending'}`;
    black.textContent = entry.black || '…';
    row.append(number, white, black);
    historyListElement.append(row);
  });
  historyListElement.scrollTop = historyListElement.scrollHeight;
}

function onSquareClick(index) {
  if (state.awaiting || state.gameOver || !state.ws || state.ws.readyState !== WebSocket.OPEN) return;
  if (state.selected === null) {
    const piece = state.board[index];
    if (!piece || !piece.startsWith('white-')) {
      setStatus('Выберите белую фигуру');
      return;
    }
    state.selected = index;
    renderBoard();
    return;
  }

  const from = state.selected;
  if (index === from) state.selected = null;
  else if (state.board[index]?.startsWith('white-')) state.selected = index;
  else sendPlayerMove(from, index);
  renderBoard();
}

function sendPlayerMove(from, to) {
  const movingPiece = state.board[from];
  let promotion;
  if (movingPiece === 'white-pawn' && Math.floor(to / 8) === 0) {
    promotion = window.prompt('Превратить пешку в: queen, rook, bishop или knight', 'queen');
    if (promotion === null) {
      state.selected = null;
      return;
    }
    promotion = promotion.trim().toLowerCase();
    if (!['queen', 'rook', 'bishop', 'knight'].includes(promotion)) {
      setStatus('Превращение: queen, rook, bishop или knight');
      state.selected = null;
      return;
    }
  }

  const move = { from, to, promotion: promotion || 'queen' };
  const payload = { type: 'player', move: [from, to] };
  if (promotion && promotion !== 'queen') payload.promotion = promotion;
  try {
    state.ws.send(JSON.stringify(payload));
  } catch {
    setStatus('Не удалось отправить ход');
    return;
  }
  state.pendingMove = move;
  state.awaiting = true;
  state.selected = null;
  state.turn = 'black';
  updateTurn();
  renderBoard();
  setStatus(`Ваш ход: ${coordinate(from)} → ${coordinate(to)}`);
}

function applyMove(move) {
  const { from, to } = move;
  const moving = state.board[from];
  if (!moving || !Number.isInteger(from) || !Number.isInteger(to) || from < 0 || from > 63 || to < 0 || to > 63) return;

  const color = moving.startsWith('white-') ? 'white' : 'black';
  const type = moving.slice(color.length + 1);
  const fromRow = Math.floor(from / 8);
  const toRow = Math.floor(to / 8);
  const toCol = to % 8;

  // En passant: pawn captures diagonally onto the square behind the moved pawn.
  if (type === 'pawn' && from % 8 !== toCol && !state.board[to]) {
    const capturedIndex = color === 'white' ? to + 8 : to - 8;
    if (state.board[capturedIndex]?.endsWith('-pawn')) state.board[capturedIndex] = null;
  }
  // Castling: king moves two squares; move the rook to the square beside it.
  if (type === 'king' && Math.abs((from % 8) - toCol) === 2) {
    const rookFrom = toCol > from % 8 ? fromRow * 8 + 7 : fromRow * 8;
    const rookTo = toCol > from % 8 ? fromRow * 8 + 5 : fromRow * 8 + 3;
    state.board[rookTo] = state.board[rookFrom];
    state.board[rookFrom] = null;
  }
  state.board[to] = type === 'pawn' && (toRow === 0 || toRow === 7)
    ? `${color}-${move.promotion || 'queen'}` : moving;
  state.board[from] = null;
  state.lastMove = { from, to };
}

function handleMessage(payload) {
  if (!payload || typeof payload !== 'object') {
    setStatus('Сервер прислал некорректные данные');
    return;
  }
  if (payload.type === 'ai' && Array.isArray(payload.move) && payload.move.length === 2) {
    if (state.pendingMove) {
      recordHistoryMove(state.pendingMove, 'white');
      applyMove(state.pendingMove);
    }
    const aiMove = { from: payload.move[0], to: payload.move[1], promotion: payload.promotion };
    recordHistoryMove(aiMove, 'black');
    applyMove(aiMove);
    state.pendingMove = null;
    state.awaiting = false;
    state.turn = 'white';
    updateTurn();
    renderBoard();
    setStatus(`Ход движка: ${coordinate(payload.move[0])} → ${coordinate(payload.move[1])}`);
    return;
  }
  if (payload.type === 'bad move') {
    state.pendingMove = null;
    state.awaiting = false;
    state.turn = 'white';
    updateTurn();
    renderBoard();
    setStatus('Сервер отклонил ход — попробуйте ещё раз');
    return;
  }
  if (payload.type === 'result' && typeof payload.result === 'string') {
    // A terminal player move receives a result directly, without an AI reply.
    if (state.pendingMove) {
      recordHistoryMove(state.pendingMove, 'white');
      applyMove(state.pendingMove);
    }
    state.pendingMove = null;
    state.awaiting = false;
    state.gameOver = true;
    renderBoard();
    const results = { win: 'Вы выиграли', lost: 'Вы проиграли', draw: 'Ничья', stalemate: 'Пат' };
    setStatus(`Партия окончена: ${results[payload.result] || payload.result}`);
    return;
  }
  setStatus('Неизвестный ответ сервера');
}

function connect() {
  if (state.ws && state.ws.readyState < WebSocket.CLOSING) state.ws.close();
  resetPosition();
  const url = wsUrlInput.value.trim() || 'ws://127.0.0.1:8080';
  setStatus('Подключение…', { label: 'Подключение', className: 'offline' });
  let socket;
  try {
    socket = new WebSocket(url);
  } catch (error) {
    setStatus(`Ошибка подключения: ${error.message}`, { label: 'Ошибка', className: 'offline' });
    return;
  }
  state.ws = socket;
  socket.addEventListener('open', () => {
    if (state.ws !== socket) return;
    setStatus('Подключено к серверу', { label: 'Подключено', className: 'online' });
    renderBoard();
  });
  socket.addEventListener('message', (event) => {
    if (state.ws !== socket) return;
    try { handleMessage(JSON.parse(event.data)); }
    catch { setStatus('Сервер прислал некорректный JSON'); }
  });
  socket.addEventListener('close', () => {
    if (state.ws !== socket) return;
    state.ws = null;
    state.awaiting = false;
    state.pendingMove = null;
    renderBoard();
    setStatus('Соединение закрыто', { label: 'Оффлайн', className: 'offline' });
  });
  socket.addEventListener('error', () => {
    if (state.ws === socket) setStatus('Ошибка WebSocket', { label: 'Ошибка', className: 'offline' });
  });
  renderBoard();
}

function newGame() {
  if (state.ws && state.ws.readyState < WebSocket.CLOSING) state.ws.close();
  state.ws = null;
  resetPosition();
  setStatus('Новая партия готова — подключитесь к серверу', { label: 'Оффлайн', className: 'offline' });
}

connectBtn.addEventListener('click', connect);
newGameBtn.addEventListener('click', newGame);
updateTurn();
renderBoard();
