from __future__ import annotations

import json
import queue
import socket
import threading
import tkinter as tk
from dataclasses import dataclass
from datetime import datetime
from tkinter import messagebox, ttk
from typing import Any

try:
    import can
except ImportError as exc:
    raise SystemExit(
        "python-can is not installed.\n"
        "Activate the WSL environment and run:\n"
        "python -m pip install python-can"
    ) from exc


# =============================================================================
# Configuration
# =============================================================================

UART_HOST = "127.0.0.1"
UART_PORT = 12345
CAN_CHANNEL = "vcan0"

CAN_ID_IGNITION_COMMAND = 0x100
CAN_ID_BCM_STATUS = 0x200

IGNITION_COMMANDS = {
    "OFF": 0,
    "ACCESSORY": 1,
    "RUNNING": 2,
}

VEHICLE_STATE_NAMES = {
    0: "OFF",
    1: "ACCESSORY",
    2: "RUNNING",
}

DOOR_STATE_NAMES = {
    0: "CLOSED",
    1: "OPEN",
}

BATTERY_STATE_NAMES = {
    0: "NORMAL",
    1: "LOW",
    2: "CRITICAL",
    3: "OVER-VOLTAGE",
}


@dataclass(frozen=True)
class BCMStatus:
    vehicle_state: int
    door_state: int
    battery_mv: int
    fault_mask: int

    @property
    def vehicle_name(self) -> str:
        return VEHICLE_STATE_NAMES.get(
            self.vehicle_state,
            f"UNKNOWN ({self.vehicle_state})",
        )

    @property
    def door_name(self) -> str:
        return DOOR_STATE_NAMES.get(
            self.door_state,
            f"UNKNOWN ({self.door_state})",
        )


class BCMCanGui:
    # -------------------------------------------------------------------------
    # Theme
    # -------------------------------------------------------------------------
    BG = "#0b1219"
    PANEL = "#16222d"
    PANEL_ALT = "#1d2b38"
    CARD = "#213140"
    CARD_DARK = "#101a23"
    TEXT = "#f0f6fb"
    MUTED = "#8fa7ba"
    ACCENT = "#18a7ff"
    SUCCESS = "#39c978"
    WARNING = "#f3b63f"
    DANGER = "#ff5b6e"
    BORDER = "#324656"
    TABLE_BG = "#0d161e"
    TABLE_ROW = "#13212b"

    VEHICLE_COLORS = {
        "OFF": "#71808d",
        "ACCESSORY": "#e0a82e",
        "RUNNING": "#35c978",
    }

    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title("Automotive BCM | Renode SocketCAN Dashboard")
        self.root.geometry("1220x820")
        self.root.minsize(880, 650)
        self.root.configure(bg=self.BG)

        # Communication objects
        self.uart_socket: socket.socket | None = None
        self.can_bus: can.BusABC | None = None
        self.running = True
        self.disconnecting = False
        self.events: queue.Queue[tuple[str, Any]] = queue.Queue()

        # Counters
        self.tx_count = 0
        self.rx_count = 0
        self.error_count = 0

        # Smart logging state
        self.last_status: BCMStatus | None = None
        self.last_logged_status: BCMStatus | None = None
        self.last_uart_json: dict[str, Any] | None = None

        # Tk variables
        self.uart_state = tk.StringVar(value="UART DISCONNECTED")
        self.can_state = tk.StringVar(value="CAN DISCONNECTED")
        self.ignition_selection = tk.StringVar(value="OFF")

        self.vehicle_state = tk.StringVar(value="UNKNOWN")
        self.door_state = tk.StringVar(value="UNKNOWN")
        self.battery_voltage = tk.StringVar(value="--")
        self.battery_condition = tk.StringVar(value="NO DATA")
        self.fault_mask = tk.StringVar(value="0x00000000")
        self.fault_description = tk.StringVar(value="No status received")
        self.last_can_frame = tk.StringVar(value="No frame received")

        self.manual_can_id = tk.StringVar(value="0x100")
        self.manual_can_data = tk.StringVar(value="00")
        self.manual_uart_command = tk.StringVar(value="GET STATUS")

        self.tx_counter_var = tk.StringVar(value="TX  0")
        self.rx_counter_var = tk.StringVar(value="RX  0")
        self.error_counter_var = tk.StringVar(value="ERRORS  0")

        self._configure_styles()
        self._build_ui()

        self.root.after(80, self._process_events)
        self.root.protocol("WM_DELETE_WINDOW", self._close)

    # =========================================================================
    # UI construction
    # =========================================================================

    def _configure_styles(self) -> None:
        style = ttk.Style()
        try:
            style.theme_use("clam")
        except tk.TclError:
            pass

        style.configure(
            "Primary.TButton",
            padding=(14, 9),
            font=("Segoe UI Semibold", 10),
        )
        style.configure(
            "Secondary.TButton",
            padding=(12, 8),
            font=("Segoe UI Semibold", 9),
        )
        style.configure(
            "Dark.TRadiobutton",
            background=self.PANEL_ALT,
            foreground=self.TEXT,
            font=("Segoe UI Semibold", 10),
        )
        style.map(
            "Dark.TRadiobutton",
            background=[
                ("active", self.PANEL_ALT),
                ("selected", self.PANEL_ALT),
            ],
            foreground=[
                ("active", self.TEXT),
                ("selected", self.TEXT),
            ],
        )

        style.configure(
            "Dark.TNotebook",
            background=self.PANEL,
            borderwidth=0,
        )
        style.configure(
            "Dark.TNotebook.Tab",
            background=self.PANEL_ALT,
            foreground=self.MUTED,
            padding=(14, 8),
            font=("Segoe UI Semibold", 9),
        )
        style.map(
            "Dark.TNotebook.Tab",
            background=[("selected", self.CARD)],
            foreground=[("selected", self.TEXT)],
        )

        style.configure(
            "Dark.Treeview",
            background=self.TABLE_BG,
            fieldbackground=self.TABLE_BG,
            foreground=self.TEXT,
            rowheight=27,
            borderwidth=0,
            font=("Consolas", 9),
        )
        style.configure(
            "Dark.Treeview.Heading",
            background=self.PANEL_ALT,
            foreground=self.TEXT,
            relief="flat",
            font=("Segoe UI Semibold", 9),
        )
        style.map(
            "Dark.Treeview",
            background=[("selected", self.ACCENT)],
            foreground=[("selected", "#ffffff")],
        )

    def _build_ui(self) -> None:
        outer = tk.Frame(self.root, bg=self.BG, padx=18, pady=15)
        outer.pack(fill="both", expand=True)

        self._build_header(outer)

        body = tk.PanedWindow(
            outer,
            orient="horizontal",
            sashwidth=7,
            sashrelief="flat",
            bg=self.BG,
            bd=0,
        )
        body.pack(fill="both", expand=True, pady=(14, 0))

        left = tk.Frame(body, bg=self.BG)
        right = tk.Frame(body, bg=self.BG)

        body.add(left, minsize=470, stretch="always")
        body.add(right, minsize=350, stretch="always")

        self._build_status_panel(left)
        self._build_controls_panel(left)
        self._build_right_panel(right)

        self._build_footer(outer)

    def _build_header(self, parent: tk.Frame) -> None:
        header = tk.Frame(parent, bg=self.BG)
        header.pack(fill="x")

        title_area = tk.Frame(header, bg=self.BG)
        title_area.pack(side="left", fill="x", expand=True)

        tk.Label(
            title_area,
            text="AUTOMOTIVE BCM",
            bg=self.BG,
            fg=self.TEXT,
            font=("Segoe UI Semibold", 24),
        ).pack(anchor="w")

        tk.Label(
            title_area,
            text="STM32F407 • Renode • SocketCAN • Python Diagnostic GUI",
            bg=self.BG,
            fg=self.MUTED,
            font=("Segoe UI", 10),
        ).pack(anchor="w", pady=(2, 0))

        connection = tk.Frame(header, bg=self.BG)
        connection.pack(side="right")

        self.uart_indicator = tk.Label(
            connection,
            textvariable=self.uart_state,
            bg=self.BG,
            fg=self.DANGER,
            font=("Segoe UI Semibold", 9),
        )
        self.uart_indicator.grid(row=0, column=0, sticky="e", padx=(0, 12))

        self.can_indicator = tk.Label(
            connection,
            textvariable=self.can_state,
            bg=self.BG,
            fg=self.DANGER,
            font=("Segoe UI Semibold", 9),
        )
        self.can_indicator.grid(row=1, column=0, sticky="e", padx=(0, 12))

        ttk.Button(
            connection,
            text="CONNECT ALL",
            style="Primary.TButton",
            command=self.connect_all,
        ).grid(row=0, column=1, rowspan=2, padx=(0, 7))

        ttk.Button(
            connection,
            text="DISCONNECT",
            style="Secondary.TButton",
            command=self.disconnect_all,
        ).grid(row=0, column=2, rowspan=2)

    def _panel(self, parent: tk.Widget) -> tk.Frame:
        return tk.Frame(
            parent,
            bg=self.PANEL,
            highlightthickness=1,
            highlightbackground=self.BORDER,
        )

    def _build_status_panel(self, parent: tk.Frame) -> None:
        panel = self._panel(parent)
        panel.pack(fill="x", padx=(0, 7))

        tk.Label(
            panel,
            text="LIVE BCM STATUS — CAN ID 0x200",
            bg=self.PANEL,
            fg=self.TEXT,
            font=("Segoe UI Semibold", 12),
        ).pack(anchor="w", padx=17, pady=(15, 9))

        cards = tk.Frame(panel, bg=self.PANEL)
        cards.pack(fill="x", padx=11, pady=(0, 11))
        cards.grid_columnconfigure(0, weight=1)
        cards.grid_columnconfigure(1, weight=1)

        self.vehicle_card, self.vehicle_value_label = self._metric_card(
            cards,
            0,
            0,
            "VEHICLE MODE",
            self.vehicle_state,
        )
        self.door_card, self.door_value_label = self._metric_card(
            cards,
            0,
            1,
            "DRIVER DOOR",
            self.door_state,
        )
        self.battery_card, self.battery_value_label = self._metric_card(
            cards,
            1,
            0,
            "BATTERY VOLTAGE",
            self.battery_voltage,
        )
        self.fault_card, self.fault_value_label = self._metric_card(
            cards,
            1,
            1,
            "ACTIVE FAULT MASK",
            self.fault_mask,
            mono=True,
        )

        self.battery_condition_label = tk.Label(
            self.battery_card,
            textvariable=self.battery_condition,
            bg=self.CARD,
            fg=self.MUTED,
            font=("Segoe UI Semibold", 8),
        )
        self.battery_condition_label.pack(anchor="w", pady=(4, 0))

        self.fault_description_label = tk.Label(
            self.fault_card,
            textvariable=self.fault_description,
            bg=self.CARD,
            fg=self.MUTED,
            font=("Segoe UI", 8),
            wraplength=210,
            justify="left",
        )
        self.fault_description_label.pack(anchor="w", pady=(4, 0))

        last = tk.Frame(cards, bg=self.CARD_DARK, padx=13, pady=10)
        last.grid(row=2, column=0, columnspan=2, sticky="ew", padx=5, pady=5)

        tk.Label(
            last,
            text="LAST CAN FRAME",
            bg=self.CARD_DARK,
            fg=self.MUTED,
            font=("Segoe UI", 8),
        ).pack(anchor="w")

        tk.Label(
            last,
            textvariable=self.last_can_frame,
            bg=self.CARD_DARK,
            fg=self.ACCENT,
            font=("Consolas", 10, "bold"),
            wraplength=640,
            justify="left",
        ).pack(anchor="w", pady=(4, 0))

    def _metric_card(
        self,
        parent: tk.Frame,
        row: int,
        column: int,
        title: str,
        variable: tk.StringVar,
        mono: bool = False,
    ) -> tuple[tk.Frame, tk.Label]:
        card = tk.Frame(parent, bg=self.CARD, padx=14, pady=12)
        card.grid(row=row, column=column, sticky="nsew", padx=5, pady=5)

        tk.Label(
            card,
            text=title,
            bg=self.CARD,
            fg=self.MUTED,
            font=("Segoe UI", 8),
        ).pack(anchor="w")

        value_label = tk.Label(
            card,
            textvariable=variable,
            bg=self.CARD,
            fg=self.TEXT,
            font=(
                ("Consolas", 14, "bold")
                if mono
                else ("Segoe UI Semibold", 15)
            ),
        )
        value_label.pack(anchor="w", pady=(7, 0))

        return card, value_label

    def _build_controls_panel(self, parent: tk.Frame) -> None:
        panel = self._panel(parent)
        panel.pack(
            fill="both",
            expand=True,
            padx=(0, 7),
            pady=(13, 0),
        )

        tk.Label(
            panel,
            text="TRANSMISSION & DIAGNOSTIC CONTROLS",
            bg=self.PANEL,
            fg=self.TEXT,
            font=("Segoe UI Semibold", 12),
        ).pack(anchor="w", padx=17, pady=(15, 9))

        # Ignition command
        ignition = tk.Frame(panel, bg=self.PANEL_ALT, padx=15, pady=13)
        ignition.pack(fill="x", padx=17, pady=(0, 12))

        tk.Label(
            ignition,
            text="IGNITION COMMAND — ID 0x100, DLC 1",
            bg=self.PANEL_ALT,
            fg=self.MUTED,
            font=("Segoe UI", 9),
        ).pack(anchor="w", pady=(0, 8))

        ignition_row = tk.Frame(ignition, bg=self.PANEL_ALT)
        ignition_row.pack(fill="x")
        ignition_row.grid_columnconfigure(0, weight=1)

        choices = tk.Frame(ignition_row, bg=self.PANEL_ALT)
        choices.grid(row=0, column=0, sticky="w")

        for name in IGNITION_COMMANDS:
            ttk.Radiobutton(
                choices,
                text=name,
                value=name,
                variable=self.ignition_selection,
                style="Dark.TRadiobutton",
            ).pack(side="left", padx=(0, 13))

        ttk.Button(
            ignition_row,
            text="SEND IGNITION",
            style="Primary.TButton",
            command=self.send_ignition,
        ).grid(row=0, column=1, sticky="e", padx=(8, 0))

        # Manual CAN frame - responsive grid
        manual = tk.Frame(panel, bg=self.PANEL_ALT, padx=15, pady=13)
        manual.pack(fill="x", padx=17, pady=(0, 12))

        tk.Label(
            manual,
            text="MANUAL CLASSICAL CAN FRAME",
            bg=self.PANEL_ALT,
            fg=self.MUTED,
            font=("Segoe UI", 9),
        ).grid(
            row=0,
            column=0,
            columnspan=4,
            sticky="w",
            pady=(0, 8),
        )

        manual.grid_columnconfigure(0, minsize=105)
        manual.grid_columnconfigure(1, weight=1)
        manual.grid_columnconfigure(2, minsize=125)

        id_box = tk.Frame(manual, bg=self.PANEL_ALT)
        id_box.grid(row=1, column=0, sticky="ew", padx=(0, 9))
        tk.Label(
            id_box,
            text="CAN ID",
            bg=self.PANEL_ALT,
            fg=self.MUTED,
            font=("Segoe UI", 8),
        ).pack(anchor="w")
        self._dark_entry(
            id_box,
            self.manual_can_id,
            width=10,
        ).pack(fill="x", ipady=6, pady=(3, 0))

        data_box = tk.Frame(manual, bg=self.PANEL_ALT)
        data_box.grid(row=1, column=1, sticky="ew", padx=(0, 9))
        tk.Label(
            data_box,
            text="DATA HEX — 0 TO 8 BYTES",
            bg=self.PANEL_ALT,
            fg=self.MUTED,
            font=("Segoe UI", 8),
        ).pack(anchor="w")
        self._dark_entry(
            data_box,
            self.manual_can_data,
        ).pack(fill="x", ipady=6, pady=(3, 0))

        # Fixed, always-visible send button
        ttk.Button(
            manual,
            text="SEND FRAME",
            style="Primary.TButton",
            command=self.send_manual_can,
            width=14,
        ).grid(
            row=1,
            column=2,
            sticky="sew",
        )

        help_label = tk.Label(
            manual,
            text="Examples: 0x100 / 63   •   0x555 / AA   •   Empty data = DLC 0",
            bg=self.PANEL_ALT,
            fg=self.MUTED,
            font=("Segoe UI", 8),
        )
        help_label.grid(
            row=2,
            column=0,
            columnspan=3,
            sticky="w",
            pady=(8, 0),
        )

        # UART controls
        uart = tk.Frame(panel, bg=self.PANEL_ALT, padx=15, pady=13)
        uart.pack(fill="x", padx=17, pady=(0, 14))

        tk.Label(
            uart,
            text="UART SIMULATION COMMAND",
            bg=self.PANEL_ALT,
            fg=self.MUTED,
            font=("Segoe UI", 9),
        ).pack(anchor="w", pady=(0, 8))

        uart_row = tk.Frame(uart, bg=self.PANEL_ALT)
        uart_row.pack(fill="x")
        uart_row.grid_columnconfigure(0, weight=1)

        self._dark_entry(
            uart_row,
            self.manual_uart_command,
        ).grid(
            row=0,
            column=0,
            sticky="ew",
            ipady=7,
            padx=(0, 8),
        )

        ttk.Button(
            uart_row,
            text="SEND UART",
            style="Secondary.TButton",
            command=self.send_uart_from_entry,
        ).grid(row=0, column=1, padx=(0, 6))

        ttk.Button(
            uart_row,
            text="GET STATUS",
            style="Secondary.TButton",
            command=lambda: self.send_uart("GET STATUS"),
        ).grid(row=0, column=2)

    def _dark_entry(
        self,
        parent: tk.Widget,
        variable: tk.StringVar,
        width: int | None = None,
    ) -> tk.Entry:
        return tk.Entry(
            parent,
            textvariable=variable,
            width=width,
            bg="#0c151d",
            fg=self.TEXT,
            insertbackground=self.TEXT,
            relief="flat",
            highlightthickness=1,
            highlightbackground=self.BORDER,
            highlightcolor=self.ACCENT,
            font=("Consolas", 11),
        )

    def _build_right_panel(self, parent: tk.Frame) -> None:
        panel = self._panel(parent)
        panel.pack(fill="both", expand=True, padx=(7, 0))

        header = tk.Frame(panel, bg=self.PANEL)
        header.pack(fill="x", padx=14, pady=(13, 7))

        tk.Label(
            header,
            text="COMMUNICATION MONITOR",
            bg=self.PANEL,
            fg=self.TEXT,
            font=("Segoe UI Semibold", 12),
        ).pack(side="left")

        ttk.Button(
            header,
            text="CLEAR",
            style="Secondary.TButton",
            command=self._clear_logs,
        ).pack(side="right")

        counters = tk.Frame(panel, bg=self.PANEL)
        counters.pack(fill="x", padx=14, pady=(0, 7))

        for variable, color in (
            (self.tx_counter_var, self.ACCENT),
            (self.rx_counter_var, self.SUCCESS),
            (self.error_counter_var, self.DANGER),
        ):
            tk.Label(
                counters,
                textvariable=variable,
                bg=self.PANEL,
                fg=color,
                font=("Segoe UI Semibold", 9),
            ).pack(side="left", padx=(0, 17))

        notebook = ttk.Notebook(panel, style="Dark.TNotebook")
        notebook.pack(fill="both", expand=True, padx=13, pady=(0, 13))

        event_tab = tk.Frame(notebook, bg=self.TABLE_BG)
        traffic_tab = tk.Frame(notebook, bg=self.TABLE_BG)
        notebook.add(event_tab, text="EVENT LOG")
        notebook.add(traffic_tab, text="CAN TRAFFIC")

        self._build_event_log(event_tab)
        self._build_traffic_table(traffic_tab)

    def _build_event_log(self, parent: tk.Frame) -> None:
        log_frame = tk.Frame(parent, bg=self.TABLE_BG)
        log_frame.pack(fill="both", expand=True)

        scrollbar = tk.Scrollbar(log_frame)
        scrollbar.pack(side="right", fill="y")

        self.log_text = tk.Text(
            log_frame,
            bg=self.TABLE_BG,
            fg="#d7e9f5",
            insertbackground=self.TEXT,
            relief="flat",
            wrap="word",
            font=("Consolas", 9),
            padx=11,
            pady=10,
            yscrollcommand=scrollbar.set,
        )
        self.log_text.pack(fill="both", expand=True)
        scrollbar.configure(command=self.log_text.yview)

        self.log_text.tag_configure("time", foreground=self.MUTED)
        self.log_text.tag_configure("tx", foreground=self.ACCENT)
        self.log_text.tag_configure("rx", foreground=self.SUCCESS)
        self.log_text.tag_configure("sys", foreground="#aab8c5")
        self.log_text.tag_configure("warn", foreground=self.WARNING)
        self.log_text.tag_configure("error", foreground=self.DANGER)
        self.log_text.tag_configure(
            "heading",
            foreground=self.TEXT,
            font=("Consolas", 9, "bold"),
        )
        self.log_text.configure(state="disabled")

    def _build_traffic_table(self, parent: tk.Frame) -> None:
        container = tk.Frame(parent, bg=self.TABLE_BG)
        container.pack(fill="both", expand=True)

        columns = ("time", "direction", "id", "dlc", "data")
        self.traffic_tree = ttk.Treeview(
            container,
            columns=columns,
            show="headings",
            style="Dark.Treeview",
        )

        self.traffic_tree.heading("time", text="TIME")
        self.traffic_tree.heading("direction", text="DIR")
        self.traffic_tree.heading("id", text="CAN ID")
        self.traffic_tree.heading("dlc", text="DLC")
        self.traffic_tree.heading("data", text="DATA")

        self.traffic_tree.column("time", width=85, anchor="center", stretch=False)
        self.traffic_tree.column(
            "direction",
            width=55,
            anchor="center",
            stretch=False,
        )
        self.traffic_tree.column("id", width=80, anchor="center", stretch=False)
        self.traffic_tree.column("dlc", width=48, anchor="center", stretch=False)
        self.traffic_tree.column("data", width=260, anchor="w", stretch=True)

        scrollbar = ttk.Scrollbar(
            container,
            orient="vertical",
            command=self.traffic_tree.yview,
        )
        self.traffic_tree.configure(yscrollcommand=scrollbar.set)

        self.traffic_tree.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        self.traffic_tree.tag_configure("TX", foreground=self.ACCENT)
        self.traffic_tree.tag_configure("RX", foreground=self.SUCCESS)

    def _build_footer(self, parent: tk.Frame) -> None:
        footer = tk.Frame(parent, bg=self.BG)
        footer.pack(fill="x", pady=(8, 0))

        tk.Label(
            footer,
            text="GUI → SocketCAN vcan0 → Renode CAN Hub → STM32F407 BCM",
            bg=self.BG,
            fg=self.MUTED,
            font=("Segoe UI", 8),
        ).pack(side="left")

        tk.Label(
            footer,
            text="BCM Diagnostic Dashboard v2.0",
            bg=self.BG,
            fg=self.MUTED,
            font=("Segoe UI", 8),
        ).pack(side="right")

    # =========================================================================
    # Connections
    # =========================================================================

    def connect_all(self) -> None:
        self.disconnecting = False
        self._connect_uart()
        self._connect_can()

    def _connect_uart(self) -> None:
        if self.uart_socket is not None:
            return

        try:
            sock = socket.create_connection(
                (UART_HOST, UART_PORT),
                timeout=2.0,
            )
            sock.settimeout(0.5)
            self.uart_socket = sock

            self.uart_state.set(
                f"● UART CONNECTED  {UART_HOST}:{UART_PORT}"
            )
            self.uart_indicator.configure(fg=self.SUCCESS)
            self._event_log(
                "SYS",
                f"UART connected to {UART_HOST}:{UART_PORT}",
            )

            threading.Thread(
                target=self._uart_receive_loop,
                args=(sock,),
                daemon=True,
            ).start()

        except OSError as exc:
            self._record_error(f"UART connection failed: {exc}")
            messagebox.showerror(
                "UART connection",
                f"Could not connect to {UART_HOST}:{UART_PORT}.\n\n{exc}",
            )

    def _connect_can(self) -> None:
        if self.can_bus is not None:
            return

        try:
            bus = can.Bus(
                interface="socketcan",
                channel=CAN_CHANNEL,
            )
            self.can_bus = bus

            self.can_state.set(f"● CAN CONNECTED  {CAN_CHANNEL}")
            self.can_indicator.configure(fg=self.SUCCESS)
            self._event_log(
                "SYS",
                f"SocketCAN connected to {CAN_CHANNEL}",
            )

            threading.Thread(
                target=self._can_receive_loop,
                args=(bus,),
                daemon=True,
            ).start()

        except (can.CanError, OSError) as exc:
            self.can_bus = None
            self._record_error(f"SocketCAN connection failed: {exc}")
            messagebox.showerror(
                "SocketCAN connection",
                f"Could not open {CAN_CHANNEL}.\n\n{exc}\n\n"
                "Create the interface with:\n"
                "sudo modprobe vcan\n"
                "sudo ip link add dev vcan0 type vcan\n"
                "sudo ip link set up vcan0",
            )

    def disconnect_all(self) -> None:
        self.disconnecting = True

        uart_socket = self.uart_socket
        self.uart_socket = None

        if uart_socket is not None:
            try:
                uart_socket.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            try:
                uart_socket.close()
            except OSError:
                pass

        can_bus = self.can_bus
        self.can_bus = None

        if can_bus is not None:
            try:
                can_bus.shutdown()
            except (can.CanError, OSError):
                pass

        self.uart_state.set("● UART DISCONNECTED")
        self.can_state.set("● CAN DISCONNECTED")
        self.uart_indicator.configure(fg=self.DANGER)
        self.can_indicator.configure(fg=self.DANGER)

        self._event_log("SYS", "Connections closed")
        self.root.after(
            250,
            lambda: setattr(self, "disconnecting", False),
        )

    # =========================================================================
    # CAN transmission
    # =========================================================================

    def send_ignition(self) -> None:
        selection = self.ignition_selection.get()
        command = IGNITION_COMMANDS[selection]

        if self._send_can(
            CAN_ID_IGNITION_COMMAND,
            bytes([command]),
        ):
            self._event_log(
                "TX",
                f"Ignition command sent → {selection}",
            )

    def send_manual_can(self) -> None:
        try:
            arbitration_id = int(
                self.manual_can_id.get().strip(),
                0,
            )

            cleaned = (
                self.manual_can_data.get()
                .replace(" ", "")
                .replace("_", "")
                .replace("-", "")
            )

            if len(cleaned) % 2 != 0:
                raise ValueError(
                    "CAN data must contain complete byte pairs."
                )

            data = bytes.fromhex(cleaned) if cleaned else b""

            if len(data) > 8:
                raise ValueError(
                    "Classical CAN supports a maximum of 8 data bytes."
                )

            if not 0 <= arbitration_id <= 0x7FF:
                raise ValueError(
                    "Use an 11-bit standard CAN ID from 0x000 to 0x7FF."
                )

        except ValueError as exc:
            messagebox.showerror("Invalid CAN frame", str(exc))
            return

        if self._send_can(arbitration_id, data):
            self._event_log(
                "TX",
                (
                    f"Manual CAN frame sent\n"
                    f"    ID   : 0x{arbitration_id:03X}\n"
                    f"    DLC  : {len(data)}\n"
                    f"    Data : {self._format_data(data)}"
                ),
            )

    def _send_can(
        self,
        arbitration_id: int,
        data: bytes,
    ) -> bool:
        bus = self.can_bus

        if bus is None:
            messagebox.showwarning(
                "CAN disconnected",
                "Connect SocketCAN first.",
            )
            return False

        message = can.Message(
            arbitration_id=arbitration_id,
            data=data,
            is_extended_id=False,
        )

        try:
            bus.send(message, timeout=1.0)
            self.tx_count += 1
            self._update_counters()
            self._add_traffic_row(
                "TX",
                arbitration_id,
                data,
            )

            # Useful when UART is present; harmless if disconnected.
            self.root.after(
                120,
                lambda: self.send_uart(
                    "GET STATUS",
                    quiet=True,
                ),
            )
            return True

        except (can.CanError, OSError) as exc:
            self._record_error(
                f"CAN transmission failed: {exc}"
            )
            messagebox.showerror(
                "CAN transmission",
                str(exc),
            )
            return False

    # =========================================================================
    # UART transmission
    # =========================================================================

    def send_uart_from_entry(self) -> None:
        self.send_uart(self.manual_uart_command.get())

    def send_uart(
        self,
        command: str,
        quiet: bool = False,
    ) -> None:
        command = command.strip()
        if not command:
            return

        sock = self.uart_socket
        if sock is None:
            if not quiet:
                messagebox.showwarning(
                    "UART disconnected",
                    "Connect UART first.",
                )
            return

        try:
            sock.sendall((command + "\n").encode("ascii"))
            if not quiet:
                self._event_log(
                    "TX",
                    f"UART command sent → {command}",
                )
        except OSError as exc:
            if not self.disconnecting:
                self._record_error(
                    f"UART send failed: {exc}"
                )

    # =========================================================================
    # Receive threads
    # =========================================================================

    def _uart_receive_loop(
        self,
        sock: socket.socket,
    ) -> None:
        buffer = ""

        while self.running and sock is self.uart_socket:
            try:
                chunk = sock.recv(4096)

                if not chunk:
                    break

                buffer += chunk.decode(
                    "utf-8",
                    errors="replace",
                )

                while "\n" in buffer:
                    line, buffer = buffer.split("\n", 1)
                    line = line.rstrip("\r")

                    if line:
                        self.events.put(("uart", line))

            except socket.timeout:
                continue

            except OSError:
                break

        self.events.put(("uart_closed", sock))

    def _can_receive_loop(
        self,
        bus: can.BusABC,
    ) -> None:
        while self.running and bus is self.can_bus:
            try:
                message = bus.recv(timeout=0.5)

            except (can.CanError, OSError) as exc:
                # Closing a CAN bus during disconnect can interrupt recv().
                # Do not report that normal shutdown as a user-visible error.
                if (
                    self.running
                    and not self.disconnecting
                    and bus is self.can_bus
                ):
                    self.events.put(
                        (
                            "can_error",
                            f"CAN connection lost: {exc}",
                        )
                    )
                break

            if message is not None:
                self.events.put(("can", message))

        self.events.put(("can_closed", bus))

    # =========================================================================
    # Event processing
    # =========================================================================

    def _process_events(self) -> None:
        while True:
            try:
                event, payload = self.events.get_nowait()
            except queue.Empty:
                break

            if event == "uart":
                self._handle_uart_line(str(payload))

            elif event == "can":
                self._handle_can_message(payload)

            elif event == "uart_closed":
                self._handle_uart_closed(payload)

            elif event == "can_closed":
                self._handle_can_closed(payload)

            elif event == "can_error":
                self._record_error(str(payload))

        if self.running:
            self.root.after(80, self._process_events)

    def _handle_uart_closed(
        self,
        closed_socket: object,
    ) -> None:
        if closed_socket is not self.uart_socket:
            return

        self.uart_socket = None
        self.uart_state.set("● UART DISCONNECTED")
        self.uart_indicator.configure(fg=self.DANGER)

        if not self.disconnecting:
            self._event_log(
                "WARN",
                "UART connection closed",
            )

    def _handle_can_closed(
        self,
        closed_bus: object,
    ) -> None:
        if closed_bus is not self.can_bus:
            return

        self.can_bus = None
        self.can_state.set("● CAN DISCONNECTED")
        self.can_indicator.configure(fg=self.DANGER)

        if not self.disconnecting:
            self._event_log(
                "WARN",
                "CAN connection closed",
            )

    # =========================================================================
    # Incoming data handlers
    # =========================================================================

    def _handle_uart_line(self, line: str) -> None:
        if not line.startswith("{"):
            # Keep important startup and response text, but avoid noise.
            if line not in ("CAN OK", "GPIO OK"):
                self._event_log("RX", f"UART → {line}")
            return

        try:
            status = json.loads(line)
        except json.JSONDecodeError:
            self._event_log(
                "WARN",
                f"Invalid UART JSON → {line}",
            )
            return

        # Log JSON only when it changes.
        if status != self.last_uart_json:
            self.last_uart_json = status
            self._event_log(
                "RX",
                (
                    "UART status response\n"
                    f"    Vehicle : "
                    f"{VEHICLE_STATE_NAMES.get(status.get('vehicle_state'), 'UNKNOWN')}\n"
                    f"    Door    : "
                    f"{DOOR_STATE_NAMES.get(status.get('door_state'), 'UNKNOWN')}\n"
                    f"    Battery : {status.get('battery_mv', 0)} mV\n"
                    f"    Faults  : 0x{status.get('fault_mask', 0):08X}"
                ),
            )

        self._apply_status_values(
            vehicle=int(status.get("vehicle_state", 0)),
            door=int(status.get("door_state", 0)),
            battery_mv=int(status.get("battery_mv", 0)),
            fault_mask=int(status.get("fault_mask", 0)),
        )

    def _handle_can_message(
        self,
        message: object,
    ) -> None:
        if not isinstance(message, can.Message):
            return

        data = bytes(message.data)
        self.rx_count += 1
        self._update_counters()

        self._add_traffic_row(
            "RX",
            message.arbitration_id,
            data,
        )

        frame_text = (
            f"ID=0x{message.arbitration_id:03X} "
            f"DLC={len(data)} "
            f"DATA={self._format_data(data)}"
        )
        self.last_can_frame.set(frame_text)

        if (
            message.arbitration_id == CAN_ID_BCM_STATUS
            and len(data) == 8
        ):
            vehicle = data[0]
            door = data[1]
            battery_mv = data[2] | (data[3] << 8)
            fault_mask = int.from_bytes(
                data[4:8],
                byteorder="little",
                signed=False,
            )

            status = BCMStatus(
                vehicle_state=vehicle,
                door_state=door,
                battery_mv=battery_mv,
                fault_mask=fault_mask,
            )

            self.last_status = status
            self._apply_status_values(
                vehicle,
                door,
                battery_mv,
                fault_mask,
            )

            # The dashboard still updates for every periodic 0x200 frame,
            # but the event log records only meaningful value changes.
            if status != self.last_logged_status:
                changed_fields = self._describe_status_changes(
                    self.last_logged_status,
                    status,
                )
                self._event_log(
                    "RX",
                    (
                        "BCM status changed\n"
                        f"    Vehicle : {status.vehicle_name}\n"
                        f"    Door    : {status.door_name}\n"
                        f"    Battery : {status.battery_mv / 1000:.2f} V\n"
                        f"    Faults  : 0x{status.fault_mask:08X}\n"
                        f"    Changed : {changed_fields}"
                    ),
                )
                self.last_logged_status = status

    def _describe_status_changes(
        self,
        previous: BCMStatus | None,
        current: BCMStatus,
    ) -> str:
        if previous is None:
            return "Initial status"

        changes: list[str] = []

        if previous.vehicle_state != current.vehicle_state:
            changes.append("vehicle")
        if previous.door_state != current.door_state:
            changes.append("door")
        if previous.battery_mv != current.battery_mv:
            changes.append("battery")
        if previous.fault_mask != current.fault_mask:
            changes.append("fault mask")

        return ", ".join(changes) if changes else "none"

    def _apply_status_values(
        self,
        vehicle: int,
        door: int,
        battery_mv: int,
        fault_mask: int,
    ) -> None:
        vehicle_name = VEHICLE_STATE_NAMES.get(
            vehicle,
            f"UNKNOWN ({vehicle})",
        )
        door_name = DOOR_STATE_NAMES.get(
            door,
            f"UNKNOWN ({door})",
        )

        self.vehicle_state.set(vehicle_name)
        self.door_state.set(door_name)
        self.battery_voltage.set(
            f"{battery_mv / 1000:.2f} V\n{battery_mv} mV"
        )
        self.fault_mask.set(f"0x{fault_mask:08X}")

        self._style_vehicle(vehicle_name)
        self._style_door(door_name)
        self._style_battery(battery_mv)
        self._style_fault(fault_mask)

    # =========================================================================
    # Dynamic styling
    # =========================================================================

    def _set_card_background(
        self,
        card: tk.Frame,
        color: str,
    ) -> None:
        card.configure(bg=color)

        for child in card.winfo_children():
            try:
                child.configure(bg=color)
            except tk.TclError:
                pass

    def _style_vehicle(self, name: str) -> None:
        color = self.VEHICLE_COLORS.get(name, self.CARD)
        self._set_card_background(self.vehicle_card, color)
        self.vehicle_value_label.configure(fg="#ffffff")

    def _style_door(self, name: str) -> None:
        if name == "OPEN":
            color = "#8b5d22"
        elif name == "CLOSED":
            color = "#205f49"
        else:
            color = self.CARD

        self._set_card_background(self.door_card, color)
        self.door_value_label.configure(fg="#ffffff")

    def _style_battery(self, battery_mv: int) -> None:
        if battery_mv <= 0:
            condition = "NO DATA"
            color = self.CARD
            accent = self.MUTED
        elif battery_mv < 9000:
            condition = "CRITICAL"
            color = "#762d39"
            accent = "#ffdce1"
        elif battery_mv < 11000:
            condition = "LOW"
            color = "#76531d"
            accent = "#ffe8b5"
        elif battery_mv > 15500:
            condition = "OVER-VOLTAGE"
            color = "#762d39"
            accent = "#ffdce1"
        else:
            condition = "NORMAL"
            color = "#205f49"
            accent = "#d7ffea"

        self.battery_condition.set(condition)
        self._set_card_background(self.battery_card, color)
        self.battery_value_label.configure(fg="#ffffff")
        self.battery_condition_label.configure(fg=accent)

    def _style_fault(self, fault_mask: int) -> None:
        if fault_mask == 0:
            self.fault_description.set("✓ No active faults")
            color = "#205f49"
            accent = "#d7ffea"
        else:
            active_bits = [
                str(bit)
                for bit in range(32)
                if fault_mask & (1 << bit)
            ]
            self.fault_description.set(
                "⚠ Active bits: " + ", ".join(active_bits)
            )
            color = "#762d39"
            accent = "#ffdce1"

        self._set_card_background(self.fault_card, color)
        self.fault_value_label.configure(fg="#ffffff")
        self.fault_description_label.configure(fg=accent)

    # =========================================================================
    # Logging and traffic monitor
    # =========================================================================

    def _event_log(
        self,
        category: str,
        message: str,
    ) -> None:
        timestamp = datetime.now().strftime("%H:%M:%S")

        tag_map = {
            "TX": "tx",
            "RX": "rx",
            "SYS": "sys",
            "WARN": "warn",
            "ERROR": "error",
        }
        tag = tag_map.get(category, "sys")

        self.log_text.configure(state="normal")
        self.log_text.insert(
            "end",
            f"[{timestamp}] ",
            ("time",),
        )
        self.log_text.insert(
            "end",
            f"{category:<5} ",
            (tag, "heading"),
        )
        self.log_text.insert(
            "end",
            message + "\n\n",
            (tag,),
        )
        self.log_text.see("end")
        self.log_text.configure(state="disabled")

    def _add_traffic_row(
        self,
        direction: str,
        arbitration_id: int,
        data: bytes,
    ) -> None:
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]

        self.traffic_tree.insert(
            "",
            "end",
            values=(
                timestamp,
                direction,
                f"0x{arbitration_id:03X}",
                len(data),
                self._format_data(data),
            ),
            tags=(direction,),
        )

        # Keep memory and GUI rendering bounded.
        rows = self.traffic_tree.get_children()
        if len(rows) > 1000:
            for row in rows[:200]:
                self.traffic_tree.delete(row)

        self.traffic_tree.yview_moveto(1.0)

    def _record_error(self, message: str) -> None:
        self.error_count += 1
        self._update_counters()
        self._event_log("ERROR", message)

    def _update_counters(self) -> None:
        self.tx_counter_var.set(f"TX  {self.tx_count}")
        self.rx_counter_var.set(f"RX  {self.rx_count}")
        self.error_counter_var.set(
            f"ERRORS  {self.error_count}"
        )

    def _clear_logs(self) -> None:
        self.log_text.configure(state="normal")
        self.log_text.delete("1.0", "end")
        self.log_text.configure(state="disabled")

        for item in self.traffic_tree.get_children():
            self.traffic_tree.delete(item)

    @staticmethod
    def _format_data(data: bytes) -> str:
        return data.hex(" ").upper() if data else "<EMPTY>"

    # =========================================================================
    # Shutdown
    # =========================================================================

    def _close(self) -> None:
        self.running = False
        self.disconnect_all()
        self.root.after(100, self.root.destroy)


def main() -> None:
    root = tk.Tk()
    BCMCanGui(root)
    root.mainloop()


if __name__ == "__main__":
    main()