#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
} pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool creates_cycle(int winner, int loser);
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{

    // Loop over the candidates and see if there is a match vs user provided name
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcmp(name, candidates[i]) == 0)
        {
            // Update the candidate (for some reason they decided to convert candiates to ints)
            // with the user perference.
            // i in this case represents the integer value of the name
            // ranks is the list of names in integer form
            // rank represents the perference of the user.

            ranks[rank] = i;
            return true;
        }
    }

    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{

    // Loop over the provided vote (this is called a lot of times in main)
    // For each candidate outside of the most perfered, add one to the count.
    // Increase I and repeat for each other vote.

    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            // Find the current vote at ranks i (for example 3 is the first, most perfered
            // candidate) Mark all the other candidates that are less perfered than I. Increase the
            // count and mark all the votes that are less perfered than the second perference and so
            // on. The least perfered candidate of that voter should then have the most marks
            // against their name.
            preferences[ranks[i]][ranks[j]]++;
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    for (int i = 0; i < candidate_count; i++)
    {
        // Int name value of the winner and loser
        int winner = 0;
        int loser = 0;

        for (int j = i + 1; j < candidate_count; j++)
        {
            // For each pairing, check if candidate i or j is perfered (we want the candidate
            // reference which is the row/column index of the matrix.
            if (preferences[i][j] > preferences[j][i])
            {
                winner = i;
                loser = j;
            }
            else if (preferences[i][j] < preferences[j][i])
            {
                winner = j;
                loser = i;
            }
            else
            {
                continue;
            }

            // Create a struct for each pair and note the winner and loser
            pair result;
            result.winner = winner;
            result.loser = loser;

            // Add the struct to the pairs list and increase the pairs count (we use it as an index)
            pairs[pair_count] = result;
            pair_count++;
        }
    }
    // TODO
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    // Set the order using selection sort
    for (int i = 0; i < pair_count; i++)
    {
        // Record the index of the greatest value
        int largest_index = i;

        for (int j = i + 1; j < pair_count; j++)
        {
            int winner_votes_i = preferences[pairs[i].winner][pairs[i].loser];
            int loser_votes_i = preferences[pairs[i].loser][pairs[i].winner];

            int winner_votes_j = preferences[pairs[j].winner][pairs[j].loser];
            int loser_votes_j = preferences[pairs[j].loser][pairs[j].winner];

            // Compare the margin of victory - ignore draws
            if (winner_votes_i - loser_votes_i < winner_votes_j - loser_votes_j)
            {
                largest_index = j;
            }
        }

        // If needed, move the pair with the largest margin to the front
        if (largest_index != i)
        {
            pair new_largest_victory = pairs[largest_index];
            pair old_largest_victory = pairs[i];

            pairs[i] = new_largest_victory;
            pairs[largest_index] = old_largest_victory;
        }
    }

    return;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{

    // The 0th index should be the root and the winner of the election
    for (int i = 0; i < candidate_count; i++)
    {
        // Check if a cycle is created by adding the winner and loser pair
        if (!creates_cycle(pairs[i].winner, pairs[i].loser))
        {
            // If the creates cycle returns false then add an edge between the pair.
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
    return;
}

// Helper function
bool creates_cycle(int winner, int loser)
// Takes 2 arguments, winner and the loser involved with the pair
{
    // Base case - there is a cycle created return true.
    if (winner == loser)
    {
        return true;
    }

    // Loop over each candidate
    for (int i = 0; i < candidate_count; i++)
    {
        // Check if there is a locked pair that exists between the two
        if (locked[loser][i])
        {
            // Call recursively call this to check for edges against all others
            if (creates_cycle(winner, i))
            {
                return true;
            }
        }
    }
    // Return false is no edge is detected
    return false;
}

// Print the winner of the election
void print_winner(void)
{
    for (int i = 0; i < candidate_count; i++)
    {
        bool winner = true;

        for (int j = 0; j < candidate_count; j++)
        {
            if (locked[j][i])
            {
                winner = false;
            }
        }

        if (winner)
        {
            string winner_name = candidates[i];
            printf("%s\n", winner_name);
        }
    }

    return;
}
