import tkinter as tk
from tkinter import messagebox

puncte_vect = []

nr_puncte = 0

def draw(event):
    x, y = event.x, event.y
    global nr_puncte
    if nr_puncte == sampling_scale.get():
        nr_puncte =0

        if puncte_vect:
            ultim_x, ultim_y = puncte_vect[-1]
            canvas.create_line(ultim_x, ultim_y, x, y, fill="green", width=3)
        puncte_vect.append((x, y))

    else:
        nr_puncte += 1


def process_points(sampling_step=1, px_to_cm=0.2):
    if not puncte_vect:
        return []
    x0, y0 = puncte_vect[0]
    relative_points = [(x - x0, y - y0) for x, y in puncte_vect]
    converted = [(round(dx * px_to_cm, 2), round(dy * px_to_cm, 2)) for dx, dy in relative_points]
    sampled_points = converted[::sampling_step]
    return sampled_points

def save_for_arduino():
    if not puncte_vect:
        messagebox.showwarning("Atenție", "Nu ai desenat nimic.")
        return

    with open("traiectorie_arduino.h", "w") as f:
        f.write("float traseu[][2] = {\n")
        for x_cm, y_cm in puncte_vect:
            f.write(f"  {{{x_cm*px_to_cm}, {(-y_cm+height)*px_to_cm}}},\n")
        f.write("};\n")
        f.write(f"int numarPuncte = sizeof(traseu) / sizeof(traseu[0]);\n")

    messagebox.showinfo("Succes", f"Fișierul 'traiectorie_arduino.h' a fost salvat cu {len(puncte_vect)} puncte.")


def clear_canvas():
    canvas.delete("all")
    puncte_vect.clear()
    print("Canvas golit.")
    draw_grid(canvas, width, height)
    draw_axes(canvas, width, height)


def draw_grid(canvas, width, height, grid_spacing=20):
    # Draw vertical grid lines and x-axis labels
    for x in range(0, width, grid_spacing):
        canvas.create_line(x, 0, x, height, fill="#ddd")
        x_val = x
        if x_val % 50 == 0:
            canvas.create_text(x, height - 15, text=str(x_val), anchor="n", fill="blue", font=("Arial", 8))

    # Draw horizontal grid lines and y-axis labels
    for y in range(0, height, grid_spacing):
        canvas.create_line(0, y, width, y, fill="#ddd")
        y_val = height - y
        if y_val % 50 == 0:
            canvas.create_text( 10, y, text=str(y_val), anchor="w", fill="red", font=("Arial", 8))

def draw_axes(canvas, width, height):
    # Draw X and Y axes in the middle
    canvas.create_line(0, height, width, height, fill="black", width=2)  # X-axis
    canvas.create_line(4, 0, 4, height, fill="black", width=2)  # Y-axis

# Inițializare interfață
root = tk.Tk()

root.title("Desenează traiectorie robot")
frame = tk.Frame(root)
frame.pack(fill=tk.BOTH, expand=True)
left_panel = tk.Frame(frame)
left_panel.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

tk.Button(left_panel, width=10, height = 8,text="Exportă",font = ("Helvetica"), command=save_for_arduino).grid(row=0, column=0, pady=5)
tk.Button(left_panel, width=10, height = 8,text="Șterge",font = "Helvetica", command=clear_canvas).grid(row=1, column=0, pady=5)

# Slider sub-eșantionare
tk.Label(left_panel, text="Rezoluție traseu:", font = "Helvetica").grid(row=2, column=0, columnspan=1)
sampling_scale = tk.Scale(left_panel, from_=1, to=100, orient=tk.HORIZONTAL)
sampling_scale.set(20)
sampling_scale.grid(row=3, column=0, columnspan=1)


width = 1300
height = 600
canvas = tk.Canvas(frame, width=width, height=height, bg="white")

canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
canvas.bind("<B1-Motion>", draw)
draw_grid(canvas, width, height)
draw_axes(canvas, width, height)

# Entry pentru conversie px -> cm

px_to_cm = 0.1


root.mainloop()
