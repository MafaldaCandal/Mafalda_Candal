# -*- coding: utf-8 -*-
"""
Created on Thu May 30 21:55:58 2024

@author: mafam
"""

import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
import random
import json
import os
import webbrowser
from datetime import datetime, timedelta

app = tk.Tk()
app.title("Task Manager")
app.geometry("1000x700")  # Set a larger fixed size for the window

# File to store tasks
tasks_file = "tasks.json"

# Initialize the data structure with categories
tasks = {
    "Urgent and Important": [],
    "Not Urgent and Important": []
}
completed_tasks = []

# Flag to track if the user has completed their first task
first_task_completed = False

def load_tasks():
    """Load tasks from the JSON file if it exists."""
    if os.path.exists(tasks_file):
        with open(tasks_file, 'r') as file:
            data = json.load(file)
            global first_task_completed
            first_task_completed = data.get('first_task_completed', False)
            return data.get('tasks', tasks), data.get('completed_tasks', completed_tasks)
    return tasks, completed_tasks

def save_tasks():
    """Save tasks and completed tasks to the JSON file."""
    data = {
        'tasks': tasks,
        'completed_tasks': completed_tasks,
        'first_task_completed': first_task_completed
    }
    with open(tasks_file, 'w') as file:
        json.dump(data, file)

def calculate_urgency(due_date):
    """Calculate the urgency level based on the due date."""
    today = datetime.now().date()
    days_remaining = (due_date - today).days
    
    if days_remaining < 2:
        return "Critical"
    elif days_remaining < 7:
        return "High"
    elif days_remaining < 30:
        return "Medium"
    else:
        return "Low"

def sort_tasks():
    """Sort tasks by urgency and timestamp (newest to oldest)."""
    urgency_order = {"Critical": 1, "High": 2, "Medium": 3, "Low": 4}
    for category in tasks:
        for task in tasks[category]:
            task['urgency'] = calculate_urgency(datetime.strptime(task['due_date'], "%Y-%m-%d").date())
        tasks[category].sort(key=lambda x: (urgency_order[x['urgency']], -datetime.strptime(x['timestamp'], "%Y-%m-%d %H:%M:%S").timestamp()))

tasks, completed_tasks = load_tasks()

def update_treeview():
    """Update the Treeview widget with current tasks or placeholder text."""
    sort_tasks()
    tree.delete(*tree.get_children())
    for category in tasks:
        parent = tree.insert('', 'end', iid=category, text=category, open=True)
        if tasks[category]:
            for task in tasks[category]:
                tree.insert(parent, 'end', values=(task['description'], task["urgency"], task['due_date'], f"{task['completion']}%", f"{task['hours_remaining']} hrs"))

def update_completed_treeview():
    """Update the Treeview widget with completed tasks."""
    completed_tree.delete(*completed_tree.get_children())
    for task in completed_tasks:
        completed_tree.insert('', 'end', values=(task['description'], task["urgency"]))

def add_task():
    """Prompt user to add a task after selecting the category."""
    category = prompt_for_category("adding")
    if category:
        description = simpledialog.askstring("Input", f"Enter a task for '{category}':", parent=app)
        due_date_str = simpledialog.askstring("Input", f"Enter due date for '{description}' (YYYY-MM-DD):", parent=app)
        hours_remaining = simpledialog.askfloat("Input", f"Enter estimated hours remaining for '{description}':", parent=app)
        try:
            due_date = datetime.strptime(due_date_str, "%Y-%m-%d").date()
        except ValueError:
            messagebox.showerror("Invalid Date", "The date format is invalid. Please enter the date in YYYY-MM-DD format.")
            return
        if description and due_date and hours_remaining is not None:
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            urgency = calculate_urgency(due_date)
            task = {"description": description, "urgency": urgency, "due_date": due_date_str, "completion": 0, "hours_remaining": hours_remaining, "timestamp": timestamp}
            tasks[category].append(task)
            update_treeview()
            save_tasks()
            messagebox.showinfo("Success", f"Task added to {category}: {description}")

def eliminate_task():
    """Prompt user to remove a task after selecting the category."""
    category = prompt_for_category("removing")
    if category:
        if not tasks[category]:
            messagebox.showinfo("No Tasks", "There are currently no tasks in this category.")
            return

        task_description = prompt_for_task("removing", category)
        if task_description:
            task_to_remove = next((task for task in tasks[category] if task["description"] == task_description), None)
            if task_to_remove:
                tasks[category].remove(task_to_remove)
                update_treeview()
                save_tasks()
                messagebox.showinfo("Success", f"Task removed from {category}: {task_description}")

def update_task_progress():
    """Update progress of a task."""
    global first_task_completed
    category = prompt_for_category("updating")
    if category:
        if not tasks[category]:
            messagebox.showinfo("No Tasks", "There are currently no tasks in this category.")
            return

        task_description = prompt_for_task("updating", category)
        if task_description:
            task_to_update = next((task for task in tasks[category] if task["description"] == task_description), None)
            if task_to_update:
                completion = simpledialog.askinteger("Input", f"Enter new completion percentage for '{task_description}':", parent=app, minvalue=0, maxvalue=100)
                hours_remaining = simpledialog.askfloat("Input", f"Enter new estimated hours remaining for '{task_description}':", parent=app)
                if completion is not None and hours_remaining is not None:
                    task_to_update["completion"] = completion
                    task_to_update["hours_remaining"] = hours_remaining

                    if completion == 100:
                        tasks[category].remove(task_to_update)
                        completed_tasks.append(task_to_update)
                        if not first_task_completed:
                            first_task_completed = True
                            webbrowser.open("https://www.youtube.com/shorts/SXHMnicI6Pg")
                    
                    update_treeview()
                    update_completed_treeview()
                    save_tasks()
                    messagebox.showinfo("Success", f"Task progress updated for {category}: {task_description}")

def select_random_tasks():
    """Display randomly selected tasks with time allocations."""
    urgent_task = random.choice(tasks["Urgent and Important"])["description"] if tasks["Urgent and Important"] else "No tasks available"
    not_urgent_task = random.choice(tasks["Not Urgent and Important"])["description"] if tasks["Not Urgent and Important"] else "No tasks available"
    messagebox.showinfo("Tasks for Today", f"Today you will dedicate 20 minutes to '{urgent_task}' and 10 minutes to '{not_urgent_task}'.")

def prompt_for_category(operation):
    """Prompt user to choose a category using buttons instead of typing."""
    def close_category_selector(return_value):
        nonlocal category
        category = return_value
        selector.destroy()

    category = None
    selector = tk.Toplevel(app)
    selector.title(f"Select Category for {operation}")
    selector.geometry("300x200")

    ttk.Label(selector, text="Choose a category:").pack(pady=20)

    for cat in tasks.keys():
        ttk.Button(selector, text=cat, command=lambda c=cat: close_category_selector(c)).pack(fill='x', padx=20, pady=10)

    selector.wait_window()
    return category

def prompt_for_task(operation, category):
    """Prompt user to choose a task from a list using buttons instead of typing."""
    def close_task_selector(return_value):
        nonlocal task
        task = return_value
        selector.destroy()

    task = None
    selector = tk.Toplevel(app)
    selector.title(f"Select Task for {operation}")
    selector.geometry("400x300")

    ttk.Label(selector, text=f"Choose a task to {operation} from '{category}':").pack(pady=20)

    for task in tasks[category]:
        ttk.Button(selector, text=task["description"], command=lambda t=task["description"]: close_task_selector(t)).pack(fill='x', padx=20, pady=10)

    selector.wait_window()
    return task

# Styling and Layout adjustments
style = ttk.Style()
style.theme_use('clam')
style.configure('TButton', font=('Helvetica', 12), padding=6)
style.configure('Accent.TButton', font=('Helvetica', 14, 'bold'))  # Bold style for the random task button

# Layout Frames
button_frame = ttk.Frame(app, padding="3 12")
button_frame.pack(fill='x')

tree_frame = ttk.Frame(app)
tree_frame.pack(fill='both', expand=True)

legend_frame = ttk.Frame(app, padding="10")
legend_frame.pack(fill='x', pady=10)

completed_frame = tk.Frame(app)
completed_frame.pack(fill='x', pady=10)

def toggle_completed_tasks():
    if completed_tree_frame.winfo_ismapped():
        completed_tree_frame.pack_forget()
        toggle_button.config(text="Show Completed Tasks")
    else:
        completed_tree_frame.pack(fill='x', pady=10)
        toggle_button.config(text="Hide Completed Tasks")

toggle_button = ttk.Button(completed_frame, text="Show Completed Tasks", command=toggle_completed_tasks)
toggle_button.pack()

completed_tree_frame = ttk.Frame(app)
completed_tree = ttk.Treeview(completed_tree_frame, columns=("Description", "Urgency"), show="headings")
completed_tree.pack(expand=True, fill='both', padx=20, pady=20)
completed_tree.heading("Description", text="Description")
completed_tree.heading("Urgency", text="Urgency")

# Treeview for task display with additional columns for task name, urgency, completion status, and hours remaining
tree = ttk.Treeview(tree_frame, columns=("Description", "Urgency", "Due Date", "Completion", "Hours Remaining"), show="headings")
tree.pack(expand=True, fill='both', padx=20, pady=20)
tree.heading("Description", text="Description")
tree.heading("Urgency", text="Urgency")
tree.heading("Due Date", text="Due Date")
tree.heading("Completion", text="Completion")
tree.heading("Hours Remaining", text="Hours Remaining")

# Task management buttons
add_button = ttk.Button(button_frame, text="Add Task", command=add_task)
add_button.pack(side='left', padx=20, pady=10)

remove_button = ttk.Button(button_frame, text="Remove Task", command=eliminate_task)
remove_button.pack(side='left', padx=20, pady=10)

update_button = ttk.Button(button_frame, text="Update Task Progress", command=update_task_progress)
update_button.pack(side='left', padx=20, pady=10)

# Special button for random tasks at the bottom
random_button = ttk.Button(app, text="Select Random Tasks for Today", command=select_random_tasks, style='Accent.TButton')
random_button.pack(fill='x', padx=50, pady=20)  # Emphasized button, now at the bottom of the main app window

# Legend for urgency levels
legend_label = ttk.Label(legend_frame, text="Urgency Levels: Low = more than 2 months, Medium = less than a month, High = less than a week, Critical = less than 2 days")
legend_label.pack()

update_treeview()
update_completed_treeview()

# Ensure tasks are saved when the application is closed
app.protocol("WM_DELETE_WINDOW", lambda: [save_tasks(), app.destroy()])

app.mainloop()
